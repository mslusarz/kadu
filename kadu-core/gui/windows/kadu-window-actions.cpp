/*
 * %kadu copyright begin%
 * Copyright 2010 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2009, 2009 Wojciech Treter (juzefwt@gmail.com)
 * Copyright 2009, 2009, 2010 Rafał Malinowski (rafal.przemyslaw.malinowski@gmail.com)
 * Copyright 2009 Michał Podsiadlik (michal@kadu.net)
 * Copyright 2009 Longer (longer89@gmail.com)
 * Copyright 2009, 2010 Piotr Galiszewski (piotrgaliszewski@gmail.com)
 * Copyright 2009 Bartłomiej Zimoń (uzi18@o2.pl)
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

#include <QtCore/QLocale>
#include <QtGui/QApplication>
#include <QtGui/QClipboard>
#include <QtGui/QInputDialog>
#include <QtGui/QMenu>

#include "accounts/account.h"
#include "accounts/account-manager.h"
#include "buddies/buddy-kadu-data.h"
#include "buddies/buddy-manager.h"
#include "buddies/buddy-shared.h"
#include "buddies/group-manager.h"
#include "configuration/configuration-file.h"
#include "contacts/contact.h"
#include "buddies/filter/has-description-buddy-filter.h"
#include "buddies/filter/offline-buddy-filter.h"
#include "buddies/filter/online-and-description-buddy-filter.h"
#include "core/core.h"
#include "gui/actions/action.h"
#include "gui/actions/actions.h"
#include "gui/widgets/buddies-list-view.h"
#include "gui/widgets/buddies-list-view-menu-manager.h"
#include "gui/widgets/chat-widget-actions.h"
#include "gui/widgets/chat-widget-manager.h"
#include "gui/widgets/status-menu.h"
#include "gui/windows/add-buddy-window.h"
#include "gui/windows/buddy-data-window.h"
#include "gui/windows/kadu-window.h"
#include "gui/windows/main-configuration-window.h"
#include "gui/windows/merge-buddies-window.h"
#include "gui/windows/message-dialog.h"
#include "gui/windows/search-window.h"
#include "gui/windows/your-accounts.h"
#include "misc/misc.h"
#include "parser/parser.h"
#include "status/status-changer-manager.h"
#include "status/status-container-manager.h"
#include "url-handlers/url-handler-manager.h"

#include "about.h"
#include "debug.h"
#include "ignore.h"
#include "modules.h"

#include "kadu-window-actions.h"

void disableNonIdUles(Action *action)
{
	kdebugf();
	foreach (const Contact &contact, action->contacts())
		if (contact.isNull())
		{
			action->setEnabled(false);
			return;
		}

	action->setEnabled(true);
	kdebugf2();
}

void disableContainsSelfUles(Action *action)
{
	if (action->buddies().contains(Core::instance()->myself()))
	{
		action->setEnabled(false);
		return;
	}

	action->setEnabled(true);
}

void checkOfflineTo(Action *action)
{
	kdebugf();
	bool on = true;
	foreach (const Buddy buddy, action->buddies())
		if (!buddy.isOfflineTo())
		{
			on = false;
			break;
		}
	action->setChecked(on);
	kdebugf2();
}

void checkHideDescription(Action *action)
{
	action->setEnabled(true);

	bool on = false;
	foreach (const Buddy buddy, action->buddies())
	{
		BuddyKaduData *ckd = 0;
		if (buddy.data())
			ckd = buddy.data()->moduleStorableData<BuddyKaduData>("kadu", false);
		if (!ckd)
			continue;

		if (ckd->hideDescription())
		{
			on = true;
			break;
		}
	}

	action->setChecked(on);
}

void disableNotOneUles(Action *action)
{
	kdebugf();

	if (action->contact().isNull())
	{
		action->setEnabled(false);
		return;
	}

	action->setEnabled(true);
	kdebugf2();
}

void disableNoGaduUle(Action *action)
{
	kdebugf();

	Contact contact = action->contact();

	if (contact.isNull())
	{
		action->setEnabled(false);
		return;
	}

	action->setEnabled(true);
	kdebugf2();
}

void disableNoGaduDescription(Action *action)
{
	kdebugf();

	Contact contact = action->contact();

	if (contact.isNull())
	{
		action->setEnabled(false);
		return;
	}

	if (contact.currentStatus().description().isEmpty())
	{
		action->setEnabled(false);
		return;
	}

	action->setEnabled(true);
	kdebugf2();
}

void disableNoGaduDescriptionUrl(Action *action)
{
	kdebugf();

	Contact contact = action->contact();

	if (contact.isNull())
	{
		action->setEnabled(false);
		return;
	}

	Status status = contact.currentStatus();
	if (status.description().isEmpty())
	{
		action->setEnabled(false);
		return;
	}

	if (status.description().indexOf(UrlHandlerManager::instance()->urlRegExp()) < 0)
	{
		action->setEnabled(false);
		return;
	}

	action->setEnabled(true);
	kdebugf2();
}

void disableNoEMail(Action *action)
{
	kdebugf();

	if (action->contacts().count() != 1)
	{
		action->setEnabled(false);
		return;
	}

	const Buddy buddy = action->contact().ownerBuddy();

	if (buddy.email().isEmpty() || buddy.email().indexOf(UrlHandlerManager::instance()->mailRegExp()) < 0)
	{
		action->setEnabled(false);
		return;
	}

	action->setEnabled(true);
	kdebugf2();
}

KaduWindowActions::KaduWindowActions(QObject *parent) : QObject(parent)
{
	Configuration = new ActionDescription(this,
		ActionDescription::TypeGlobal, "configurationAction",
		this, SLOT(configurationActionActivated(QAction *, bool)),
		"16x16/preferences-other.png", "16x16/preferences-other.png", tr("Configure Kadu...")
	);
	Configuration->setShortcut("kadu_configure", Qt::ApplicationShortcut);

	ShowYourAccounts = new ActionDescription(this,
		ActionDescription::TypeMainMenu, "yourAccountsAction",
		this, SLOT(yourAccountsActionActivated(QAction *, bool)),
		"16x16/x-office-address-book.png", "16x16/x-office-address-book.png", tr("Your accounts...")
	);

	ManageModules = new ActionDescription(this,
		ActionDescription::TypeMainMenu, "manageModulesAction",
		ModulesManager::instance(), SLOT(showDialog(QAction *, bool)),
		"kadu_icons/kadu-modmanager.png", "kadu_icons/kadu-modmanager.png", tr("Plugins...")
	);
	ManageModules->setShortcut("kadu_modulesmanager", Qt::ApplicationShortcut);

	ExitKadu = new ActionDescription(this,
		ActionDescription::TypeMainMenu, "exitKaduAction",
		this, SLOT(exitKaduActionActivated(QAction *, bool)),
		"16x16/application-exit.png", "16x16/application-exit.png", tr("&Exit")
	);
	ExitKadu->setShortcut("kadu_exit", Qt::ApplicationShortcut);

	AddUser = new ActionDescription(this,
		ActionDescription::TypeGlobal, "addUserAction",
		this, SLOT(addUserActionActivated(QAction *, bool)),
		"16x16/contact-new.png", "16x16/contact-new.png", tr("Add Buddy...")
	);
	AddUser->setShortcut("kadu_adduser", Qt::ApplicationShortcut);
	BuddiesListViewMenuManager::instance()->insertManagementActionDescription(0, AddUser);

	MergeContact = new ActionDescription(this,
		ActionDescription::TypeUser, "mergeContactAction",
		this, SLOT(mergeContactActionActivated(QAction *, bool)),
		"", "", tr("Merge Buddy...")
	);
	BuddiesListViewMenuManager::instance()->insertManagementActionDescription(1, MergeContact);

	BuddiesListViewMenuManager::instance()->insertManagementActionDescription(2, 0);

	AddGroup= new ActionDescription(this,
		ActionDescription::TypeGlobal, "addGroupAction",
		this, SLOT(addGroupActionActivated(QAction *, bool)),
		//TODO 0.6.6 proper icon
		"16x16/contact-new.png", "16x16/contact-new.png", tr("Add Group...")
	);

	OpenSearch = new ActionDescription(this,
		ActionDescription::TypeGlobal, "openSearchAction",
		this, SLOT(openSearchActionActivated(QAction *, bool)),
		"16x16/edit-find.png", "16x16/edit-find.png", tr("Search for Buddies...")
	);

	ManageIgnored = new ActionDescription(this,
		ActionDescription::TypeMainMenu, "manageIgnoredAction",
		this, SLOT(manageIgnoredActionActivated(QAction *, bool)),
		"kadu_icons/kadu-manageignored.png", "kadu_icons/kadu-manageignored.png", tr("Ignored Buddies...")
	);

	Help = new ActionDescription(this,
		ActionDescription::TypeMainMenu, "helpAction",
		this, SLOT(helpActionActivated(QAction *, bool)),
		"16x16/help-contents.png", "16x16/help-contents.png", tr("Getting H&elp")
	);

	Bugs = new ActionDescription(this,
		ActionDescription::TypeMainMenu, "bugsAction",
		this, SLOT(bugsActionActivated(QAction *, bool)),
		"", "", tr("Submitt Bug Report")
	);

	Support = new ActionDescription(this,
		ActionDescription::TypeMainMenu, "supportAction",
		this, SLOT(supportActionActivated(QAction *, bool)),
		"", "", tr("Support us")
	);

	GetInvolved = new ActionDescription(this,
		ActionDescription::TypeMainMenu, "getInvolvedAction",
		this, SLOT(getInvolvedActionActivated(QAction *, bool)),
		"", "", tr("Get Involved")
	);

	About = new ActionDescription(this,
		ActionDescription::TypeMainMenu, "aboutAction",
		this, SLOT(aboutActionActivated(QAction *, bool)),
		"16x16/help-about.png", "16x16/help-about.png", tr("A&bout Kadu")
	);

	CopyDescription = new ActionDescription(this,
		ActionDescription::TypeUser, "copyDescriptionAction",
		this, SLOT(copyDescriptionActionActivated(QAction *, bool)),
		"16x16/edit-copy.png", "16x16/edit-copy.png", tr("Copy description"), false, "",
		disableNoGaduDescription
	);
	BuddiesListViewMenuManager::instance()->addListActionDescription(CopyDescription);

	CopyPersonalInfo = new ActionDescription(this,
		ActionDescription::TypeUser, "copyPersonalInfoAction",
		this, SLOT(copyPersonalInfoActionActivated(QAction *, bool)),
		"kadu_icons/kadu-copypersonal.png", "kadu_icons/kadu-copypersonal.png", tr("Copy personal info")
	);
	BuddiesListViewMenuManager::instance()->addListActionDescription(CopyPersonalInfo);

	OpenDescriptionLink = new ActionDescription(this,
		ActionDescription::TypeUser, "openDescriptionLinkAction",
		this, SLOT(openDescriptionLinkActionActivated(QAction *, bool)),
		"16x16/go-jump.png", "16x16/go-jump.png", tr("Open description link in browser"), false, "",
		disableNoGaduDescriptionUrl
	);
	BuddiesListViewMenuManager::instance()->addListActionDescription(OpenDescriptionLink);

	WriteEmail = new ActionDescription(this,
		ActionDescription::TypeUser, "writeEmailAction",
		this, SLOT(writeEmailActionActivated(QAction *, bool)),
		"16x16/mail-message-new.png", "16x16/mail-message-new.png", tr("Write email message"), false, "",
		disableNoEMail
	);
	BuddiesListViewMenuManager::instance()->addListActionDescription(WriteEmail);

	LookupUserInfo = new ActionDescription(this,
		ActionDescription::TypeUser, "lookupUserInfoAction",
		this, SLOT(lookupInDirectoryActionActivated(QAction *, bool)),
		"16x16/edit-find.png", "16x16/edit-find.png", tr("Search in directory"), false, "",
		disableNoGaduUle
	);

	BuddiesListViewMenuManager::instance()->insertManagementActionDescription(3, ChatWidgetManager::instance()->actions()->ignoreUser());
	BuddiesListViewMenuManager::instance()->insertManagementActionDescription(4, ChatWidgetManager::instance()->actions()->blockUser());

	OfflineToUser = new ActionDescription(this,
		ActionDescription::TypeUser, "offlineToUserAction",
		this, SLOT(offlineToUserActionActivated(QAction *, bool)),
		"protocols/gadu-gadu/16x16/offline.png", "protocols/gadu-gadu/16x16/offline.png", tr("Offline to user"), true, "",
		checkOfflineTo
	);
	BuddiesListViewMenuManager::instance()->addManagementActionDescription(OfflineToUser);

	HideDescription = new ActionDescription(this,
		ActionDescription::TypeUser, "hideDescriptionAction",
		this, SLOT(hideDescriptionActionActivated(QAction *, bool)),
		"kadu_icons/kadu-descriptions_on.png", "kadu_icons/kadu-descriptions_off.png", tr("Hide description"), true, "",
		checkHideDescription
	);
	BuddiesListViewMenuManager::instance()->addManagementActionDescription(HideDescription);
	BuddiesListViewMenuManager::instance()->addManagementSeparator();

	DeleteUsers = new ActionDescription(this,
		ActionDescription::TypeUser, "deleteUsersAction",
		this, SLOT(deleteUsersActionActivated(QAction *, bool)),
		"16x16/edit-delete.png", "16x16/edit-delete.png", tr("Delete")
	);
	DeleteUsers->setShortcut("kadu_deleteuser");
	BuddiesListViewMenuManager::instance()->addManagementActionDescription(DeleteUsers);

	InactiveUsers = new ActionDescription(this,
		ActionDescription::TypeUserList, "inactiveUsersAction",
		this, SLOT(inactiveUsersActionActivated(QAction *, bool)),
		"protocols/gadu-gadu/16x16/offline.png", "protocols/gadu-gadu/16x16/offline.png", tr("Hide offline users"),
		true, tr("Show offline users")
	);
	connect(InactiveUsers, SIGNAL(actionCreated(Action *)), this, SLOT(inactiveUsersActionCreated(Action *)));
	InactiveUsers->setShortcut("kadu_showoffline");

	DescriptionUsers = new ActionDescription(this,
		ActionDescription::TypeUserList, "descriptionUsersAction",
		this, SLOT(descriptionUsersActionActivated(QAction *, bool)),
		"kadu_icons/kadu-showdescriptionusers_off.png", "kadu_icons/kadu-showdescriptionusers.png", tr("Hide users without description"),
		true, tr("Show users without description")
	);
	connect(DescriptionUsers, SIGNAL(actionCreated(Action *)), this, SLOT(descriptionUsersActionCreated(Action *)));
	DescriptionUsers->setShortcut("kadu_showonlydesc");

	OnlineAndDescriptionUsers = new ActionDescription(this,
		ActionDescription::TypeUserList, "onlineAndDescriptionUsersAction",
		this, SLOT(onlineAndDescUsersActionActivated(QAction *, bool)),
		"kadu_icons/kadu-onoff_onlineandd_off.png", "kadu_icons/kadu-onoff_onlineandd.png", tr("Show only online and description users"),
		true, tr("Show all users")
	);
	connect(OnlineAndDescriptionUsers, SIGNAL(actionCreated(Action *)), this, SLOT(onlineAndDescUsersActionCreated(Action *)));

	BuddiesListViewMenuManager::instance()->addSeparator();
	BuddiesListViewMenuManager::instance()->addSeparator();

	EditUser = new ActionDescription(this,
		ActionDescription::TypeUser, "editUserAction",
		this, SLOT(editUserActionActivated(QAction *, bool)),
		"16x16/x-office-address-book", "16x16/x-office-address-book", tr("Buddy Properties"), false, QString::null,
		disableNotOneUles
	);
	connect(EditUser, SIGNAL(actionCreated(Action *)), this, SLOT(editUserActionCreated(Action *)));
	EditUser->setShortcut("kadu_persinfo");
	BuddiesListViewMenuManager::instance()->addActionDescription(EditUser);

	ShowStatus = new ActionDescription(this,
		ActionDescription::TypeGlobal, "openStatusAction",
		this, SLOT(showStatusActionActivated(QAction *, bool)),
		"protocols/gadu-gadu/16x16/offline.png", "protocols/gadu-gadu/16x16/offline.png", tr("Change status")
	);
	connect(ShowStatus, SIGNAL(actionCreated(Action *)), this, SLOT(showStatusActionCreated(Action *)));

	UseProxy = new ActionDescription(this,
		ActionDescription::TypeGlobal, "useProxyAction",
		this, SLOT(useProxyActionActivated(QAction *, bool)),
		"kadu_icons/kadu-proxy.png", "kadu_icons/kadu-proxy_off.png", tr("Use proxy"), true, tr("Don't use proxy")
	);
	connect(UseProxy, SIGNAL(actionCreated(Action *)), this, SLOT(useProxyActionCreated(Action *)));

	connect(StatusChangerManager::instance(), SIGNAL(statusChanged(StatusContainer *, Status)), this, SLOT(statusChanged(StatusContainer *, Status)));
}

KaduWindowActions::~KaduWindowActions()
{
}

void KaduWindowActions::statusChanged(StatusContainer *container, Status status)
{
	if (!container)
		return;

	// TODO: 0.6.6, this really SUXX
	QIcon icon = container->statusPixmap(status);
	foreach (Action *action, ShowStatus->actions())
		action->setIcon(icon);
}

void KaduWindowActions::inactiveUsersActionCreated(Action *action)
{
	MainWindow *window = qobject_cast<MainWindow *>(action->parent());
	if (!window)
		return;
	if (!window->contactsListView())
		return;

	bool enabled = !config_file.readBoolEntry("General", "ShowOffline");
	OfflineBuddyFilter *ofcf = new OfflineBuddyFilter(action);
	ofcf->setEnabled(enabled);

	action->setData(QVariant::fromValue(ofcf));
	action->setChecked(enabled);

	window->contactsListView()->addFilter(ofcf);
}

void KaduWindowActions::descriptionUsersActionCreated(Action *action)
{
	MainWindow *window = qobject_cast<MainWindow *>(action->parent());
	if (!window)
		return;
	if (!window->contactsListView())
		return;

	bool enabled = !config_file.readBoolEntry("General", "ShowWithoutDescription");
	HasDescriptionBuddyFilter *hdcf = new HasDescriptionBuddyFilter(action);
	hdcf->setEnabled(enabled);

	action->setData(QVariant::fromValue(hdcf));
	action->setChecked(enabled);

	window->contactsListView()->addFilter(hdcf);
}

void KaduWindowActions::onlineAndDescUsersActionCreated(Action *action)
{
	MainWindow *window = qobject_cast<MainWindow *>(action->parent());
	if (!window)
		return;
	if (!window->contactsListView())
		return;

	bool enabled = config_file.readBoolEntry("General", "ShowOnlineAndDescription");
	OnlineAndDescriptionBuddyFilter *oadcf = new OnlineAndDescriptionBuddyFilter(action);
	oadcf->setEnabled(enabled);

	action->setData(QVariant::fromValue(oadcf));
	action->setChecked(enabled);

	window->contactsListView()->addFilter(oadcf);
}

void KaduWindowActions::editUserActionCreated(Action *action)
{
	MainWindow *window = dynamic_cast<MainWindow *>(action->parent());
	if (!window)
		return;

	Buddy buddy = window->contact().ownerBuddy();
	if (buddy.isAnonymous())
	{
		action->setIcon(IconsManager::instance()->iconByPath("16x16/contact-new.png"));
		action->setText(tr("Add user"));
	}
}

void KaduWindowActions::showStatusActionCreated(Action *action)
{
	Account account = AccountManager::instance()->defaultAccount();

	if (account.protocolHandler())
		action->setIcon(account.protocolHandler()->statusPixmap());
}

void KaduWindowActions::useProxyActionCreated(Action *action)
{
	action->setChecked(config_file.readBoolEntry("Network", "UseProxy", false));
}

void KaduWindowActions::configurationActionActivated(QAction *sender, bool toggled)
{
	Q_UNUSED(sender)
	Q_UNUSED(toggled)

	MainConfigurationWindow::instance()->show();
}

void KaduWindowActions::yourAccountsActionActivated(QAction *sender, bool toggled)
{
	Q_UNUSED(sender)
	Q_UNUSED(toggled)

	(new YourAccounts())->show();
}

void KaduWindowActions::exitKaduActionActivated(QAction *sender, bool toggled)
{
	Q_UNUSED(sender)
	Q_UNUSED(toggled)

	kdebugf();

	// TODO: 0.6.6
	//if (measureTime)
	//{
	//	time_t sec;
	//	int msec;
	//	getTime(&sec, &msec);
	//	endingTime = (sec % 1000) * 1000 + msec;
	//}
	qApp->quit();
}

void KaduWindowActions::addUserActionActivated(QAction *sender, bool toggled)
{
	Q_UNUSED(toggled)

	kdebugf();

	MainWindow *window = dynamic_cast<MainWindow *>(sender->parent());
	if (!window)
		return;

	Buddy buddy = window->contact().ownerBuddy();
	AddBuddyWindow *addBuddyWindow = new AddBuddyWindow(window);

	if (buddy.isAnonymous())
		addBuddyWindow->setBuddy(buddy);

	addBuddyWindow->show();

 	kdebugf2();
}

void KaduWindowActions::mergeContactActionActivated(QAction *sender, bool toggled)
{
	Q_UNUSED(toggled)

	kdebugf();

	MainWindow *window = dynamic_cast<MainWindow *>(sender->parent());
	if (!window)
		return;

	Buddy buddy = window->contact().ownerBuddy();
	if (!buddy.isNull())
	{
		MergeBuddiesWindow *mergeBuddiesWindow = new MergeBuddiesWindow(buddy, window);
		mergeBuddiesWindow->show();
	}

	kdebugf2();
}

void KaduWindowActions::addGroupActionActivated(QAction *sender, bool toggled)
{
	Q_UNUSED(toggled)

	bool ok;
	QString newGroupName = QInputDialog::getText(dynamic_cast<QWidget *>(sender->parent()), tr("New Group"),
				tr("Please enter the name for the new group:"), QLineEdit::Normal,
				QString::null, &ok);

	if (ok && !newGroupName.isEmpty() && GroupManager::instance()->acceptableGroupName(newGroupName))
		GroupManager::instance()->byName(newGroupName);
}

void KaduWindowActions::openSearchActionActivated(QAction *sender, bool toggled)
{
	Q_UNUSED(toggled)

	(new SearchWindow(dynamic_cast<QWidget *>(sender->parent())))->show();
}

void KaduWindowActions::manageIgnoredActionActivated(QAction *sender, bool toggled)
{
	Q_UNUSED(toggled)

	(new Ignored(dynamic_cast<QWidget *>(sender->parent())))->show();
}

void KaduWindowActions::helpActionActivated(QAction *sender, bool toggled)
{
	Q_UNUSED(sender)
	Q_UNUSED(toggled)

	if (config_file.readEntry("General", "Language", QString(qApp->keyboardInputLocale().name()).mid(0,2)) == "pl")
		openWebBrowser("http://www.kadu.net/w/Pomoc_online");
	else
		openWebBrowser("http://www.kadu.net/w/English:Kadu:Help_online");
}

void KaduWindowActions::bugsActionActivated(QAction *sender, bool toggled)
{
	Q_UNUSED(sender)
	Q_UNUSED(toggled)

	if (config_file.readEntry("General", "Language", QString(qApp->keyboardInputLocale().name()).mid(0,2)) == "pl")
		openWebBrowser("http://www.kadu.net/w/B%C5%82%C4%99dy");
	else
		openWebBrowser("http://www.kadu.net/w/English:Bugs");
}

void KaduWindowActions::supportActionActivated(QAction *sender, bool toggled)
{
	Q_UNUSED(sender)
	Q_UNUSED(toggled)

	if (config_file.readEntry("General", "Language", QString(qApp->keyboardInputLocale().name()).mid(0,2)) == "pl")
		openWebBrowser("http://www.kadu.net/w/Kadu:Site_support");
	else
		openWebBrowser("http://www.kadu.net/w/English:Kadu:Site_support");
}

void KaduWindowActions::getInvolvedActionActivated(QAction *sender, bool toggled)
{
	Q_UNUSED(sender)
	Q_UNUSED(toggled)

	if (config_file.readEntry("General", "Language", QString(qApp->keyboardInputLocale().name()).mid(0,2)) == "pl")
		openWebBrowser("http://www.kadu.net/w/Do%C5%82%C4%85cz");
	else
		openWebBrowser("http://www.kadu.net/w/English:GetInvolved");
}

void KaduWindowActions::aboutActionActivated(QAction *sender, bool toggled)
{
	Q_UNUSED(sender)
	Q_UNUSED(toggled)

	(new ::About(Core::instance()->kaduWindow()))->show();
}

void KaduWindowActions::writeEmailActionActivated(QAction *sender, bool toggled)
{
	Q_UNUSED(toggled)

	kdebugf();

	MainWindow *window = dynamic_cast<MainWindow *>(sender->parent());
	if (!window)
		return;

	Buddy buddy = window->contact().ownerBuddy();
	if (buddy.isNull())
		return;

	if (!buddy.email().isEmpty())
		openMailClient(buddy.email());

	kdebugf2();
}

void KaduWindowActions::copyDescriptionActionActivated(QAction *sender, bool toggled)
{
	Q_UNUSED(toggled)

	kdebugf();

	MainWindow *window = dynamic_cast<MainWindow *>(sender->parent());
	if (!window)
		return;

	Contact data = window->contact();

	if (data.isNull())
		return;

	QString description = data.currentStatus().description();
	if (description.isEmpty())
		return;

	QApplication::clipboard()->setText(description, QClipboard::Selection);
	QApplication::clipboard()->setText(description, QClipboard::Clipboard);

	kdebugf2();
}

void KaduWindowActions::openDescriptionLinkActionActivated(QAction *sender, bool toggled)
{
	Q_UNUSED(toggled)

	kdebugf();

	MainWindow *window = dynamic_cast<MainWindow *>(sender->parent());
	if (!window)
		return;

	Contact data = window->contact();

	if (data.isNull())
		return;

	QString description = data.currentStatus().description();
	if (description.isEmpty())
		return;

	QRegExp url = UrlHandlerManager::instance()->urlRegExp();
	int idx_start = url.indexIn(description);
	if (idx_start >= 0)
		openWebBrowser(description.mid(idx_start, url.matchedLength()));

	kdebugf2();
}

void KaduWindowActions::copyPersonalInfoActionActivated(QAction *sender, bool toggled)
{
	Q_UNUSED(toggled)

	kdebugf();

	MainWindow *window = dynamic_cast<MainWindow *>(sender->parent());
	if (!window)
		return;

	BuddySet buddies = window->buddies();

	QStringList infoList;
	QString copyPersonalDataSyntax = config_file.readEntry("General", "CopyPersonalDataSyntax", tr("Contact: %a[ (%u)]\n[First name: %f\n][Last name: %r\n][Mobile: %m\n]"));
	foreach (Buddy buddy, buddies)
		infoList.append(Parser::parse(copyPersonalDataSyntax, buddy.prefferedAccount(), buddy, false));

	QString info = infoList.join("\n");
	if (info.isEmpty())
		return;

	QApplication::clipboard()->setText(info, QClipboard::Selection);
	QApplication::clipboard()->setText(info, QClipboard::Clipboard);

	kdebugf2();
}

void KaduWindowActions::lookupInDirectoryActionActivated(QAction *sender, bool toggled)
{
	Q_UNUSED(toggled)

	kdebugf();

	MainWindow *window = dynamic_cast<MainWindow *>(sender->parent());
	if (!window)
		return;

	Buddy buddy = window->contact().ownerBuddy();
	if (buddy.isNull())
		return;

	SearchWindow *sd = new SearchWindow(Core::instance()->kaduWindow(), buddy);
	sd->show();
	sd->firstSearch();

	kdebugf2();
}

void KaduWindowActions::offlineToUserActionActivated(QAction *sender, bool toggled)
{
	kdebugf();

	if (toggled && !config_file.readBoolEntry("General", "PrivateStatus"))
	{
// TODO: 0.6.6
// 		if (MessageDialog::ask("You need to have private status to do it, would you like to set private status now?"))
// 			changePrivateStatus->setChecked(true);
// 		else
// 		{
// 			sender->setChecked(!toggled);
// 			return;
// 		}
	}

	MainWindow *window = dynamic_cast<MainWindow *>(sender->parent());
	if (!window)
		return;

	BuddySet buddies = window->buddies();
	bool on = true;
	foreach (const Buddy buddy, buddies)
		if (!buddy.isOfflineTo())
		{
			on = false;
			break;
		}
	/*
	foreach(const Contact contact, contacts)
		if (contact.accountData(account) != 0 || contact.isOfflineTo(account) == on)
			//TODO: 0.6.6
			user.setProtocolData("Gadu", "OfflineTo", !on); // TODO: here boolean
	*/
// TODO: 0.6.6
// 	userlist->writeToConfig();

	foreach (Action *action, OfflineToUser->actions())
		if (action->buddies() == buddies)
			action->setChecked(!on);

	kdebugf2();
}

void KaduWindowActions::hideDescriptionActionActivated(QAction *sender, bool toggled)
{
	kdebugf();

	MainWindow *window = dynamic_cast<MainWindow *>(sender->parent());
	if (!window)
		return;

	BuddySet buddies = window->buddies();

	foreach (const Buddy &buddy, buddies)
	{
		if (buddy.isNull() || buddy.isAnonymous())
			continue;

		BuddyKaduData *bkd = 0;
		if (buddy.data())
			bkd = buddy.data()->moduleStorableData<BuddyKaduData>("kadu", true);
		if (!bkd)
			continue;

		if (bkd->hideDescription() != toggled)
		{
			bkd->setHideDescription(toggled);
			bkd->store();
		}
	}

	foreach (Action *action, HideDescription->actions())
		if (action->buddies() == buddies)
			action->setChecked(toggled);

	kdebugf2();
}

void KaduWindowActions::deleteUsersActionActivated(QAction *sender, bool toggled)
{
	Q_UNUSED(toggled)

	kdebugf();

	MainWindow *window = dynamic_cast<MainWindow *>(sender->parent());
	if (!window)
		return;

	BuddySet buddies = window->buddies();
	if (buddies.isEmpty())
		return;

	QStringList displays;
	foreach (Buddy buddy, buddies)
		displays.append(buddy.display());
	if (MessageDialog::ask(tr("Selected users:\n%0 will be deleted. Are you sure?").arg(displays.join(", ")), "32x32/dialog-warning.png", Core::instance()->kaduWindow()))
	{
		foreach (Buddy buddy, buddies)
			BuddyManager::instance()->removeItem(buddy);
		BuddyManager::instance()->store();
	}

	kdebugf2();
}

void KaduWindowActions::inactiveUsersActionActivated(QAction *sender, bool toggled)
{
	MainWindow *window = dynamic_cast<MainWindow *>(sender->parent());
	if (!window)
		return;

	QVariant v = sender->data();
	if (v.canConvert<OfflineBuddyFilter *>())
	{
		OfflineBuddyFilter *ofcf = v.value<OfflineBuddyFilter *>();
		ofcf->setEnabled(toggled);
	}
}

void KaduWindowActions::descriptionUsersActionActivated(QAction *sender, bool toggled)
{
	MainWindow *window = dynamic_cast<MainWindow *>(sender->parent());
	if (!window)
		return;

	QVariant v = sender->data();
	if (v.canConvert<HasDescriptionBuddyFilter *>())
	{
		HasDescriptionBuddyFilter *hdcf = v.value<HasDescriptionBuddyFilter *>();
		hdcf->setEnabled(toggled);
	}
}

void KaduWindowActions::onlineAndDescUsersActionActivated(QAction *sender, bool toggled)
{
	MainWindow *window = dynamic_cast<MainWindow *>(sender->parent());
	if (!window)
		return;

	QVariant v = sender->data();
	if (v.canConvert<OnlineAndDescriptionBuddyFilter *>())
	{
		OnlineAndDescriptionBuddyFilter *oadcf = v.value<OnlineAndDescriptionBuddyFilter *>();
		oadcf->setEnabled(toggled);
	}
}

void KaduWindowActions::editUserActionActivated(QAction *sender, bool toggled)
{
	Q_UNUSED(toggled)

	kdebugf();

	MainWindow *window = dynamic_cast<MainWindow *>(sender->parent());
	if (!window)
		return;

	Buddy buddy = window->contact().ownerBuddy();
	if (buddy.isNull())
		buddy = BuddyManager::instance()->byContact(window->contact(), ActionCreateAndAdd);

	if (buddy.isAnonymous())
	{
		AddBuddyWindow *addBuddyWindow = new AddBuddyWindow(window);
		addBuddyWindow->setBuddy(buddy);
		addBuddyWindow->show();
	}
	else
		(new BuddyDataWindow(buddy, window))->show();

	kdebugf2();
}

void KaduWindowActions::showStatusActionActivated(QAction *sender, bool toggled)
{
	Q_UNUSED(toggled)

	MainWindow *window = dynamic_cast<MainWindow *>(sender->parent());
	if (!window)
		return;

	StatusContainer *container = window->statusContainer();
	if (!container)
		container = StatusContainerManager::instance();

	QMenu *menu = new QMenu();
	StatusMenu *status = new StatusMenu(container, menu);
	status->addToMenu(menu);
	menu->exec(QCursor::pos());
	delete menu;
}

void KaduWindowActions::useProxyActionActivated(QAction *sender, bool toggled)
{
	Q_UNUSED(sender)

	config_file.writeEntry("Network", "UseProxy", toggled);

	foreach (Action *action, UseProxy->actions())
		action->setChecked(toggled);
}

// void Kadu::setProxyActionsStatus() TODO: 0.6.6
// {
// 	setProxyActionsStatus(config_file.readBoolEntry("Network", "UseProxy", false));
// }
