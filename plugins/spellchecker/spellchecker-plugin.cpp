/*
 * %kadu copyright begin%
 * Copyright 2011 Sławomir Stępień (s.stepien@interia.pl)
 * Copyright 2011, 2012, 2013 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2011, 2013, 2014 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include "core/application.h"
#include "core/core.h"
#include "misc/paths-provider.h"

#include "configuration/spellchecker-configuration.h"
#include "spellchecker-plugin.h"
#include "spellchecker.h"
#include "suggester.h"

SpellCheckerPlugin * SpellCheckerPlugin::Instance = 0;

SpellCheckerPlugin::SpellCheckerPlugin(QObject *parent) :
		QObject(parent), SpellCheckerInstance(0)
{
	Instance = this;
}

SpellCheckerPlugin::~SpellCheckerPlugin()
{
	Instance = 0;
}

bool SpellCheckerPlugin::init(PluginRepository *pluginRepository)
{
	Q_UNUSED(pluginRepository)

	SpellCheckerInstance = new SpellChecker(this);
	SpellCheckerInstance->setChatWidgetRepository(Core::instance()->chatWidgetRepository());

	SpellcheckerConfiguration::createInstance();

	MainConfigurationWindow::registerUiFile(Application::instance()->pathsProvider()->dataPath() + QLatin1String("plugins/configuration/spellchecker.ui"));
	MainConfigurationWindow::registerUiHandler(SpellCheckerInstance);

	return true;
}

void SpellCheckerPlugin::done()
{
	Suggester::destroyInstance();
	SpellcheckerConfiguration::destroyInstance();

	MainConfigurationWindow::unregisterUiHandler(SpellCheckerInstance);
	MainConfigurationWindow::unregisterUiFile(Application::instance()->pathsProvider()->dataPath() + QLatin1String("plugins/configuration/spellchecker.ui"));

	delete SpellCheckerInstance;
	SpellCheckerInstance = 0;
}

Q_EXPORT_PLUGIN2(spellchecker, SpellCheckerPlugin)

#include "moc_spellchecker-plugin.cpp"
