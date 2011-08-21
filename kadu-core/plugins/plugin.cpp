/*
 * %kadu copyright begin%
 * Copyright 2010 Rafał Malinowski (rafal.przemyslaw.malinowski@gmail.com)
 * %kadu copyright end%
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <QtCore/QFileInfo>
#include <QtCore/QPluginLoader>
#include <QtCore/QTimer>
#include <QtCore/QTranslator>
#include <QtGui/QApplication>

#include "configuration/configuration-file.h"
#include "gui/windows/plugin-error-dialog.h"
#include "gui/windows/modules-window.h"
#include "misc/path-conversion.h"
#include "plugins/generic-plugin.h"
#include "plugins/plugin-info.h"
#include "debug.h"

#include "plugin.h"

#if defined(Q_OS_WIN)
	#define SO_EXT "dll"
	#define SO_EXT_LEN 3
	#define SO_PREFIX ""
	#define SO_PREFIX_LEN 0
#else
	#define SO_EXT "so"
	#define SO_EXT_LEN 2
	#define SO_PREFIX "lib"
	#define SO_PREFIX_LEN 3
#endif

/**
 * @author Rafał 'Vogel' Malinowski
 * @short Creates new Plugin object and loads PluginInfo data.
 * @param name name of plugin
 * @param parent parent object, almost always PluginManager instance
 *
 * This contructor load data for plugin with given name. New instance of PluginInfo class
 * is created if dataDir/kadu/plugins/name.desc is found. If this file is not found, plugin
 * is marked as invalid and will be unable to be activated.
 */
Plugin::Plugin(const QString &name, QObject *parent) :
		QObject(parent),
		Name(name), Active(false), State(PluginStateNew), PluginLoader(0), PluginObject(0),
		Translator(0), UsageCounter(0)
{
	QString descFilePath = dataPath("kadu/plugins/" + name + ".desc");
	QFileInfo descFileInfo(descFilePath);

	if (descFileInfo.exists())
		Info = new PluginInfo(descFilePath);
	else
		Info = 0;

	StorableObject::setState(StateNotLoaded);
}

Plugin::~Plugin()
{
}

/**
 * @author Rafał 'Vogel' Malinowski
 * @short Loadas plugin data from storage.
 *
 * This method load plugin data (for now it is only State) from /Plugins/Plugin[\@name=pluginName]/ storage node.
 */
void Plugin::load()
{
	if (!isValidStorage())
		return;

	StorableObject::load();

	QString stateString = loadValue<QString>("State");
	if (stateString == "Loaded")
		State = PluginStateEnabled;
	else if (stateString == "NotLoaded")
		State = PluginStateDisabled;
	else
		State = PluginStateNew;
}

/**
 * @author Rafał 'Vogel' Malinowski
 * @short Stores plugin data to storage.
 *
 * This method stores plugin data (for now it is only State) into /Plugins/Plugin[\@name=pluginName]/ storage node.
 */
void Plugin::store()
{
	if (!isValidStorage())
		return;

	ensureLoaded();

	StorableObject::store();

	switch (State)
	{
		case PluginStateNew:
			storeValue("State", "New"); // should not happen, but who knows..
			break;
		case PluginStateEnabled:
			storeValue("State", "Loaded");
			break;
		case PluginStateDisabled:
			storeValue("State", "NotLoaded");
			break;
	}
}

/**
 * @author Rafał 'Vogel' Malinowski
 * @short Returns true if this plugin should be activated.
 * @return true if this plugin should be activated
 *
 * Module should be activated only if:
 * <ul>
 *   <li>it is valid (has .desc file associated with it)
 *   <li>is either PluginStateEnabled or PluginStateNew with PluginInfo::loadByDefault() set to true
 * </ul>
 */
bool Plugin::shouldBeActivated()
{
	ensureLoaded();

	if (!isValid() || isActive())
		return false;

	if (PluginStateEnabled == State)
		return true;
	if (PluginStateDisabled == State)
		return false;
	return Info->loadByDefault();
}

/**
 * @author Rafał 'Vogel' Malinowski
 * @short Activates plugin and retursn true if plugin is active.
 * @param reason plugin activation reason
 * @return true if plugin is active after method return
 *
 * This method loads plugin library file (if exists) and set up  GenericPlugin object from this library.
 * Then translations file is loaded. Next GenericPlugin::init() method is called with firstLoad
 * paramer set to true if this plugin's state  if PluginStateNew. If this methods returns value different
 * than 0, plugin is deactivated and false value is returned.
 *
 * Translations must be loaded before the root component of the plugin is instantiated.
 *
 * This method returns true if plugin is active after method returns - especially when plugin was active
 * before this call.
 */
bool Plugin::activate(PluginActivationReason reason)
{
	if (Active)
		return true;

	ensureLoaded();

	PluginLoader = new QPluginLoader(libPath("kadu/plugins/"SO_PREFIX + Name + "." SO_EXT));
	PluginLoader->setLoadHints(QLibrary::ExportExternalSymbolsHint);

	if (!PluginLoader->load())
	{
		QString err = PluginLoader->errorString();
		kdebugm(KDEBUG_ERROR, "cannot load %s because of: %s\n", qPrintable(Name), qPrintable(err));

		activationError(tr("Cannot load %1 plugin library.:\n%2").arg(Name, err), reason);

		kdebugf2();
		return false;
	}

	// Load translations before the root component of the plugin is instantiated (it is done by instance() method).
	loadTranslations();

	PluginObject = qobject_cast<GenericPlugin *>(PluginLoader->instance());
	if (!PluginObject)
	{
		activationError(tr("Cannot find required object in module %1.\nMaybe it's not Kadu-compatible plugin.").arg(Name), reason);

		delete PluginLoader;
		PluginLoader = 0;

		unloadTranslations();

		kdebugf2();
		return false;
	}

	int res = PluginObject->init(PluginStateNew == State);

	if (res != 0)
	{
		activationError(tr("Module initialization routine for %1 failed.").arg(Name), reason);

		PluginLoader->unload();
		delete PluginLoader;
		PluginLoader = 0;
		PluginObject = 0;

		unloadTranslations();

		return false;
	}

	UsageCounter = 0;

	/* This is perfectly intentional. We have to set state to either enabled or disabled, as new
	 * means that it was never loaded. If the only reason to load the plugin was because some other
	 * plugin depended upon it, set state to disabled as we don't want that plugin to be loaded
	 * next time when its reverse dependency will not be loaded. Otherwise set state to enabled.
	 */
	if (PluginActivationReasonDependency != reason)
		setState(PluginStateEnabled);
	else
		setState(PluginStateDisabled);

	Active = true;

	kdebugf2();

	return true;
}

/**
 * @author Rafał 'Vogel' Malinowski
 * @short Deactivates plugin.
 *
 * If plugin is active, its GenericPlugin::done() method is called and then all data is removed from
 * memory - plugin library file and plugin translations.
 */
void Plugin::deactivate(PluginDeactivationReason reason)
{
	if (!Active)
		return;

	if (PluginObject)
		PluginObject->done();

	if (PluginLoader)
	{
		PluginLoader->unload();
		PluginLoader->deleteLater();
		PluginLoader = 0;
	}
	PluginObject = 0;

	// We cannot unload translations before calling PluginObject->done(), see #2177.
	unloadTranslations();

	Active = false;

	if (PluginDeactivationReasonUserRequest == reason)
		setState(PluginStateDisabled);
}

/**
 * @author Rafał 'Vogel' Malinowski
 * @short Loads translations for plugin.
 *
 * This method loads translations for current Kadu's language. Translations are loaded from
 * dataDir/kadu/plugins/translations/pluginName_lang.qm file.
 */
void Plugin::loadTranslations()
{
	Translator = new QTranslator(this);
	const QString lang = config_file.readEntry("General", "Language");

	if (Translator->load(Name + '_' + lang, dataPath("kadu/plugins/translations/")))
		qApp->installTranslator(Translator);
	else
	{
		delete Translator;
		Translator = 0;
	}
}

/**
 * @author Rafał 'Vogel' Malinowski
 * @author Bartosz 'beevvy' Brachaczek
 * @short Unloads translations for plugin.
 *
 * This method unloads existing translations for this plugin. Translations are removed
 * from QApplication instance and \c Translator is deleted.
 */
void Plugin::unloadTranslations()
{
	if (Translator)
	{
		qApp->removeTranslator(Translator);
		delete Translator;
		Translator = 0;
	}
}

/**
 * @author Rafał 'Vogel' Malinowski
 * @short Sets state of plugin.
 *
 * This method changes state of plugin. Set state to PluginStateEnabled to make this plugin
 * activate at every Kadu run.
 *
 * Please do not call this method unless you are absolutely sure the plugin had been loaded
 * at least once.
 */
void Plugin::setState(Plugin::PluginState state)
{
	ensureLoaded();

	State = state;
}

/**
 * @author Bartosz 'beevvy' Brachaczek
 * @short Sets state enablement of plugin if it is inactive.
 *
 * If this plugin is active or its state is PluginStateNew, this method does nothing.
 *
 * Otherwise, this method sets its state to PluginStateEnabled if \p enable is true.
 * If \p enable is false, this method sets the plugin's state to PluginStateDisabled.
 */
void Plugin::setStateEnabledIfInactive(bool enable)
{
	if (isActive())
		return;

	if (PluginStateNew == state())
		return;

	setState(enable ? PluginStateEnabled : PluginStateDisabled);
}

/**
 * @author Bartosz 'beevvy' Brachaczek
 * @short Shows activation error to the user.
 * @param errorMessage error message that will be displayer to the user
 * @param activationReason plugin activation reason
 * @todo it really shouldn't call gui classes directly
 *
 * This method creates new PluginErrorDialog with message \p errorMessage and opens it. Depending on
 * \p activationReason, it also intructs the dialog wheter to offer the user choice wheter to try
 * to load this plugin automatically in future.
 */
void Plugin::activationError(const QString &errorMessage, PluginActivationReason activationReason)
{
	bool offerLoadInFutureChoice = (PluginActivationReasonKnownDefault == activationReason);

	PluginErrorDialog *errorDialog = new PluginErrorDialog(errorMessage, offerLoadInFutureChoice, ModulesWindow::instance());
	if (offerLoadInFutureChoice)
		connect(errorDialog, SIGNAL(accepted(bool)), this, SLOT(setStateEnabledIfInactive(bool)));

	QTimer::singleShot(0, errorDialog, SLOT(open()));
}
