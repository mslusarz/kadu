/*
 * %kadu copyright begin%
 * Copyright 2011 Piotr Galiszewski (piotr.galiszewski@kadu.im)
 * Copyright 2012 Wojciech Treter (juzefwt@gmail.com)
 * Copyright 2011, 2014 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2011, 2012, 2013, 2014 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#pragma once

#include <QtCore/QDateTime>
#include <QtCore/QMap>
#include <QtCore/QMutex>
#include <QtCore/QObject>
#include <QtCore/QPair>
#include <QtCore/QQueue>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtGui/QKeyEvent>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QLabel>
#include <QtWidgets/QListWidget>
#include <injeqt/injeqt.h>

#include "actions/action-description.h"
#include "actions/action.h"
#include "configuration/configuration-aware-object.h"
#include "configuration/gui/configuration-ui-handler.h"
#include "core/crash-aware-object.h"
#include "message/message.h"
#include "protocols/protocol.h"
#include "storage/history-storage.h"

#include "history-exports.h"

class AccountManager;
class Account;
class BuddyChatManager;
class ChatWidget;
class ChatWidgetRepository;
class ClearHistoryAction;
class Configuration;
class HistorySaveThread;
class HistoryWindow;
class PluginInjectedFactory;
class MenuInventory;
class MessageManager;
class ShowHistoryAction;

class HISTORYAPI History : public QObject, ConfigurationAwareObject, CrashAwareObject
{
    Q_OBJECT

    QPointer<AccountManager> m_accountManager;
    QPointer<BuddyChatManager> m_buddyChatManager;
    QPointer<ChatWidgetRepository> m_chatWidgetRepository;
    QPointer<ClearHistoryAction> m_clearHistoryAction;
    QPointer<Configuration> m_configuration;
    QPointer<PluginInjectedFactory> m_pluginInjectedFactory;
    QPointer<MenuInventory> m_menuInventory;
    QPointer<ShowHistoryAction> m_showHistoryAction;

    bool SaveChats;
    bool SaveChatsWithAnonymous;
    bool SaveStatuses;
    bool SaveOnlyStatusesWithDescription;
    bool SyncEnabled;

    int ChatHistoryCitation;
    int ChatHistoryQuotationTime;

    QMutex UnsavedDataMutex;
    QQueue<Message> UnsavedMessages;
    QQueue<QPair<Contact, Status>> UnsavedStatusChanges;
    HistorySaveThread *SaveThread;

    HistoryStorage *CurrentStorage;

    QListWidget *allStatusUsers;
    QListWidget *selectedStatusUsers;
    QListWidget *allChatsUsers;
    QListWidget *selectedChatsUsers;

    void startSaveThread();
    void stopSaveThread();

    void createDefaultConfiguration();

    void createActionDescriptions();
    void deleteActionDescriptions();
    virtual void configurationUpdated();

    friend class HistorySaveThread;
    Message dequeueUnsavedMessage();
    QPair<Contact, Status> dequeueUnsavedStatusChange();

    bool shouldSaveForBuddy(const Buddy &buddy);
    bool shouldSaveForChat(const Chat &chat);
    bool shouldEnqueueMessage(const Message &message);

private slots:
    INJEQT_SET void setAccountManager(AccountManager *accountManager);
    INJEQT_SET void setBuddyChatManager(BuddyChatManager *buddyChatManager);
    INJEQT_SET void setChatWidgetRepository(ChatWidgetRepository *chatWidgetRepository);
    INJEQT_SET void setClearHistoryAction(ClearHistoryAction *clearHistoryAction);
    INJEQT_SET void setConfiguration(Configuration *configuration);
    INJEQT_SET void setPluginInjectedFactory(PluginInjectedFactory *pluginInjectedFactory);
    INJEQT_SET void setMenuInventory(MenuInventory *menuInventory);
    INJEQT_SET void setMessageManager(MessageManager *messageManager);
    INJEQT_SET void setShowHistoryAction(ShowHistoryAction *showHistoryAction);
    INJEQT_INIT void init();
    INJEQT_DONE void done();

    void accountAdded(Account);
    void accountRemoved(Account);

    void enqueueMessage(const Message &);
    void contactStatusChanged(Contact contact, Status oldStatus);

    void chatWidgetAdded(ChatWidget *chatWidget);

protected:
    virtual void crash();

public:
    Q_INVOKABLE explicit History(QObject *parent = nullptr);
    virtual ~History();

    HistoryStorage *currentStorage()
    {
        return CurrentStorage;
    }
    void registerStorage(HistoryStorage *storage);
    void unregisterStorage(HistoryStorage *storage);

    void forceSync();
    void setSyncEnabled(bool syncEnabled);

signals:
    void storageChanged(HistoryStorage *newStorage);
};

void disableNonHistoryContacts(Action *action);
