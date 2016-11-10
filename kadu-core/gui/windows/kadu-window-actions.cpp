/*
 * %kadu copyright begin%
 * Copyright 2009, 2010, 2011 Piotr Galiszewski (piotr.galiszewski@kadu.im)
 * Copyright 2009, 2010, 2012 Wojciech Treter (juzefwt@gmail.com)
 * Copyright 2010, 2011 Piotr Dąbrowski (ultr@ultr.pl)
 * Copyright 2009 Michał Podsiadlik (michal@kadu.net)
 * Copyright 2009 Bartłomiej Zimoń (uzi18@o2.pl)
 * Copyright 2010 Radosław Szymczyszyn (lavrin@gmail.com)
 * Copyright 2010, 2011, 2012, 2013, 2014 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2009, 2010, 2011, 2012, 2013, 2014, 2015 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
 * Copyright 2009 Longer (longer89@gmail.com)
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
#include <QtGui/QClipboard>
#include <QtWidgets/QApplication>
#include <QtWidgets/QInputDialog>
#include <QtWidgets/QMenu>

#include "accounts/account-manager.h"
#include "accounts/account.h"
#include "actions/add-group-action.h"
#include "actions/add-user-action.h"
#include "actions/copy-description-action.h"
#include "actions/copy-personal-info-action.h"
#include "actions/exit-action.h"
#include "actions/lookup-buddy-info-action.h"
#include "actions/merge-buddies-action.h"
#include "actions/open-buddy-email-action.h"
#include "actions/open-description-link-action.h"
#include "actions/open-forum-action.h"
#include "actions/open-get-involved-action.h"
#include "actions/open-redmine-action.h"
#include "actions/open-search-action.h"
#include "actions/open-translate-action.h"
#include "actions/show-about-window-action.h"
#include "actions/show-blocked-buddies-action.h"
#include "actions/show-configuration-window-action.h"
#include "actions/show-descriptions-action.h"
#include "actions/show-info-panel-action.h"
#include "actions/show-multilogons-action.h"
#include "actions/show-myself-action.h"
#include "actions/show-offline-buddies-action.h"
#include "actions/show-only-buddies-with-description-action.h"
#include "actions/show-only-buddies-with-description-or-online-action.h"
#include "actions/show-your-accounts-action.h"
#include "buddies/buddy-manager.h"
#include "configuration/configuration.h"
#include "configuration/deprecated-configuration-api.h"
#include "contacts/contact.h"
#include "core/application.h"
#include "core/injected-factory.h"
#include "core/myself.h"
#include "gui/actions/action.h"
#include "gui/actions/actions.h"
#include "gui/actions/change-status-action.h"
#include "gui/actions/chat/add-conference-action.h"
#include "gui/actions/chat/add-room-chat-action.h"
#include "gui/actions/default-proxy-action.h"
#include "gui/actions/delete-talkable-action.h"
#include "gui/actions/edit-talkable-action.h"
#include "gui/actions/recent-chats-action.h"
#include "gui/actions/talkable-tree-view/collapse-action.h"
#include "gui/actions/talkable-tree-view/expand-action.h"
#include "gui/menu/menu-inventory.h"
#include "gui/status-icon.h"
#include "gui/widgets/buddy-info-panel.h"
#include "gui/widgets/chat-widget/actions/chat-widget-actions.h"
#include "gui/widgets/chat-widget/chat-widget-manager.h"
#include "gui/widgets/dialog/merge-buddies-dialog-widget.h"
#include "gui/widgets/status-menu.h"
#include "gui/widgets/talkable-delegate-configuration.h"
#include "gui/widgets/talkable-tree-view.h"
#include "gui/windows/add-buddy-window.h"
#include "gui/windows/buddy-delete-window.h"
#include "gui/windows/kadu-dialog.h"
#include "gui/windows/kadu-window-service.h"
#include "gui/windows/kadu-window.h"
#include "gui/windows/group-edit-window.h"
#include "gui/windows/main-configuration-window-service.h"
#include "gui/windows/message-dialog.h"
#include "gui/windows/multilogon-window-service.h"
#include "gui/windows/search-window.h"
#include "gui/windows/your-accounts-window-service.h"
#include "icons/kadu-icon.h"
#include "misc/misc.h"
#include "model/roles.h"
#include "os/generic/url-opener.h"
#include "parser/parser.h"
#include "protocols/protocol.h"
#include "status/status-container-manager.h"
#include "talkable/filter/blocked-talkable-filter.h"
#include "talkable/filter/hide-offline-talkable-filter.h"
#include "talkable/filter/hide-offline-without-description-talkable-filter.h"
#include "talkable/filter/hide-without-description-talkable-filter.h"
#include "talkable/model/talkable-model.h"
#include "talkable/model/talkable-proxy-model.h"
#include "url-handlers/url-handler-manager.h"

#include "about.h"
#include "debug.h"

#include "kadu-window-actions.h"

void disableNoContact(Action *action)
{
	action->setEnabled(action->context()->contacts().toContact());
}

void disableIfContactSelected(Myself *myself, Action *action)
{
	if (!action || action->context())
		return;

	action->setEnabled(!action->context()->roles().contains(ContactRole) && !action->context()->buddies().isEmpty());

	if (action->context()->buddies().contains(myself->buddy()))
		action->setEnabled(false);
	else
		action->setEnabled(true);
}

KaduWindowActions::KaduWindowActions(QObject *parent) : QObject(parent)
{
}

KaduWindowActions::~KaduWindowActions()
{
}

void KaduWindowActions::setAccountManager(AccountManager *accountManager)
{
	m_accountManager = accountManager;
}

void KaduWindowActions::setActions(Actions *actions)
{
	m_actions = actions;
}

void KaduWindowActions::setAddConferenceAction(AddConferenceAction *addConferenceAction)
{
	m_addConferenceAction = addConferenceAction;
}

void KaduWindowActions::setAddGroupAction(AddGroupAction *addGroupAction)
{
	m_addGroupAction = addGroupAction;
}

void KaduWindowActions::setAddRoomChatAction(AddRoomChatAction *addRoomChatAction)
{
	m_addRoomChatAction = addRoomChatAction;
}

void KaduWindowActions::setAddUserAction(AddUserAction *addUserAction)
{
	m_addUserAction = addUserAction;
}

void KaduWindowActions::setApplication(Application *application)
{
	m_application = application;
}

void KaduWindowActions::setChatWidgetActions(ChatWidgetActions *chatWidgetActions)
{
	m_chatWidgetActions = chatWidgetActions;
}

void KaduWindowActions::setChangeStatusAction(ChangeStatusAction *changeStatusAction)
{
	m_changeStatusAction = changeStatusAction;
}

void KaduWindowActions::setCollapseAction(CollapseAction *collapseAction)
{
	m_collapseAction = collapseAction;
}

void KaduWindowActions::setConfiguration(Configuration *configuration)
{
	m_configuration = configuration;
}

void KaduWindowActions::setCopyDescriptionAction(CopyDescriptionAction *copyDescriptionAction)
{
	m_copyDescriptionAction = copyDescriptionAction;
}

void KaduWindowActions::setCopyPersonalInfoAction(CopyPersonalInfoAction *copyPersonalInfoAction)
{
	m_copyPersonalInfoAction = copyPersonalInfoAction;
}

void KaduWindowActions::setDeleteTalkableAction(DeleteTalkableAction *deleteTalkableAction)
{
	m_deleteTalkableAction = deleteTalkableAction;
}

void KaduWindowActions::setEditTalkableAction(EditTalkableAction *editTalkableAction)
{
	m_editTalkableAction = editTalkableAction;
}

void KaduWindowActions::setExpandAction(ExpandAction *expandAction)
{
	m_expandAction = expandAction;
}

void KaduWindowActions::setExitAction(ExitAction *exitAction)
{
	m_exitAction = exitAction;
}

void KaduWindowActions::setInjectedFactory(InjectedFactory *injectedFactory)
{
	m_injectedFactory = injectedFactory;
}

void KaduWindowActions::setKaduWindowService(KaduWindowService *kaduWindowService)
{
	m_kaduWindowService = kaduWindowService;
}

void KaduWindowActions::setLookupBuddyInfoAction(LookupBuddyInfoAction *lookupBuddyInfoAction)
{
	m_lookupBuddyInfoAction = lookupBuddyInfoAction;
}

void KaduWindowActions::setMainConfigurationWindowService(MainConfigurationWindowService *mainConfigurationWindowService)
{
	m_mainConfigurationWindowService = mainConfigurationWindowService;
}

void KaduWindowActions::setMenuInventory(MenuInventory *menuInventory)
{
	m_menuInventory = menuInventory;
}

void KaduWindowActions::setMergeBuddiesAction(MergeBuddiesAction *mergeBuddiesAction)
{
	m_mergeBuddiesAction = mergeBuddiesAction;
}

void KaduWindowActions::setMultilogonWindowService(MultilogonWindowService *multilogonWindowService)
{
	m_multilogonWindowService = multilogonWindowService;
}

void KaduWindowActions::setMyself(Myself *myself)
{
	m_myself = myself;
}

void KaduWindowActions::setOpenBuddyEmailAction(OpenBuddyEmailAction *openBuddyEmailAction)
{
	m_openBuddyEmailAction = openBuddyEmailAction;
}

void KaduWindowActions::setOpenDescriptionLinkAction(OpenDescriptionLinkAction *openDescriptionLinkAction)
{
	m_openDescriptionLinkAction = openDescriptionLinkAction;
}

void KaduWindowActions::setOpenForumAction(OpenForumAction *openForumAction)
{
	m_openForumAction = openForumAction;
}

void KaduWindowActions::setOpenGetInvolvedAction(OpenGetInvolvedAction *openGetInvolvedAction)
{
	m_openGetInvolvedAction = openGetInvolvedAction;
}

void KaduWindowActions::setOpenRedmineAction(OpenRedmineAction *openRedmineAction)
{
	m_openRedmineAction = openRedmineAction;
}

void KaduWindowActions::setOpenSearchAction(OpenSearchAction *openSearchAction)
{
	m_openSearchAction = openSearchAction;
}

void KaduWindowActions::setOpenTranslateAction(OpenTranslateAction *openTranslateAction)
{
	m_openTranslateAction = openTranslateAction;
}

void KaduWindowActions::setParser(Parser *parser)
{
	m_parser = parser;
}

void KaduWindowActions::setRecentChatsAction(RecentChatsAction *recentChatsAction)
{
	m_recentChatsAction = recentChatsAction;
}

void KaduWindowActions::setShowAboutWindowAction(ShowAboutWindowAction *showAboutWindowAction)
{
	m_showAboutWindowAction = showAboutWindowAction;
}

void KaduWindowActions::setShowBlockedBuddiesAction(ShowBlockedBuddiesAction *showBlockedBuddiesAction)
{
	m_showBlockedBuddiesAction = showBlockedBuddiesAction;
}

void KaduWindowActions::setShowConfigurationWindowAction(ShowConfigurationWindowAction *showConfigurationWindowAction)
{
	m_showConfigurationWindowAction = showConfigurationWindowAction;
}

void KaduWindowActions::setShowDescriptionsAction(ShowDescriptionsAction *showDescriptionsAction)
{
	m_showDescriptionsAction = showDescriptionsAction;
}

void KaduWindowActions::setShowInfoPanelAction(ShowInfoPanelAction *showInfoPanelAction)
{
	m_showInfoPanelAction = showInfoPanelAction;
}

void KaduWindowActions::setShowMultilogonsAction(ShowMultilogonsAction *showMultilogonsAction)
{
	m_showMultilogonsAction = showMultilogonsAction;
}

void KaduWindowActions::setShowMyselfAction(ShowMyselfAction *showMyselfAction)
{
	m_showMyselfAction = showMyselfAction;
}

void KaduWindowActions::setShowOfflineBuddiesAction(ShowOfflineBuddiesAction *showOfflineBuddiesAction)
{
	m_showOfflineBuddiesAction = showOfflineBuddiesAction;
}

void KaduWindowActions::setShowOnlyBuddiesWithDescriptionAction(ShowOnlyBuddiesWithDescriptionAction *showOnlyBuddiesWithDescriptionAction)
{
	m_showOnlyBuddiesWithDescriptionAction = showOnlyBuddiesWithDescriptionAction;
}

void KaduWindowActions::setShowOnlyBuddiesWithDescriptionOrOnlineAction(ShowOnlyBuddiesWithDescriptionOrOnlineAction *showOnlyBuddiesWithDescriptionOrOnlineAction)
{
	m_showOnlyBuddiesWithDescriptionOrOnlineAction = showOnlyBuddiesWithDescriptionOrOnlineAction;
}

void KaduWindowActions::setShowYourAccountsAction(ShowYourAccountsAction *showYourAccountsAction)
{
	m_showYourAccountsAction = showYourAccountsAction;
}

void KaduWindowActions::setUrlHandlerManager(UrlHandlerManager *urlHandlerManager)
{
	m_urlHandlerManager = urlHandlerManager;
}

void KaduWindowActions::setUrlOpener(UrlOpener *urlOpener)
{
	m_urlOpener = urlOpener;
}

void KaduWindowActions::setYourAccountsWindowService(YourAccountsWindowService *yourAccountsWindowService)
{
	m_yourAccountsWindowService = yourAccountsWindowService;
}

void KaduWindowActions::init()
{
	m_actions->blockSignals();

	m_showConfigurationWindowAction->setShortcut("kadu_configure", Qt::ApplicationShortcut);
	m_exitAction->setShortcut("kadu_exit", Qt::ApplicationShortcut);
	m_addUserAction->setShortcut("kadu_adduser", Qt::ApplicationShortcut);

	m_menuInventory
		->menu("buddy-list")
		->addAction(m_expandAction, KaduMenu::SectionActionsGui, 2);
	m_menuInventory
		->menu("buddy-list")
		->addAction(m_collapseAction, KaduMenu::SectionActionsGui, 1);

	m_menuInventory
		->menu("buddy-list")
		->addAction(m_copyDescriptionAction, KaduMenu::SectionActions, 10);

	m_menuInventory
		->menu("buddy-list")
		->addAction(m_copyPersonalInfoAction, KaduMenu::SectionActions, 20);

	m_menuInventory
		->menu("buddy-list")
		->addAction(m_openDescriptionLinkAction, KaduMenu::SectionActions, 30);

	m_menuInventory
		->menu("buddy-list")
		->addAction(m_openBuddyEmailAction, KaduMenu::SectionSend, 200);

	m_showOfflineBuddiesAction->setShortcut("kadu_showoffline");
	m_showOnlyBuddiesWithDescriptionAction->setShortcut("kadu_showonlydesc");

	m_menuInventory
		->menu("buddy-list")
		->addAction(m_editTalkableAction, KaduMenu::SectionView);

	m_menuInventory
		->menu("buddy-list")
		->addAction(m_mergeBuddiesAction, KaduMenu::SectionManagement, 100);

	m_menuInventory
		->menu("buddy-list")
		->addAction(m_chatWidgetActions->blockUser(), KaduMenu::SectionManagement, 500);

	m_menuInventory
		->menu("buddy-list")
		->addAction(m_deleteTalkableAction, KaduMenu::SectionManagement, 1000);

	// The last ActionDescription will send actionLoaded() signal.
	// TODO It will not reflect all action types (see MainWindow::actionLoadedOrUnloaded() method)
	// but will work good since KaduActions is created very early. Of course we still need a better mechanism for that.
	m_actions->unblockSignals();

	DefaultProxy = m_injectedFactory->makeInjected<DefaultProxyAction>(this);
	m_actions->insert(DefaultProxy);
}

#include "moc_kadu-window-actions.cpp"
