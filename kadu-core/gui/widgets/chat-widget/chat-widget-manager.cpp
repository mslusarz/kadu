/*
 * %kadu copyright begin%
 * Copyright 2009, 2010, 2010, 2011 Piotr Galiszewski (piotr.galiszewski@kadu.im)
 * Copyright 2009, 2012 Wojciech Treter (juzefwt@gmail.com)
 * Copyright 2010 Piotr Dąbrowski (ultr@ultr.pl)
 * Copyright 2009 Michał Podsiadlik (michal@kadu.net)
 * Copyright 2009 Bartłomiej Zimoń (uzi18@o2.pl)
 * Copyright 2009, 2009, 2010, 2011, 2012, 2013 Rafał Malinowski (rafal.przemyslaw.malinowski@gmail.com)
 * Copyright 2010, 2011, 2012, 2013 Bartosz Brachaczek (b.brachaczek@gmail.com)
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

#include "chat-widget-manager.h"

#include <QtGui/QApplication>

#include "chat/buddy-chat-manager.h"
#include "chat/chat-manager.h"
#include "chat/type/chat-type-contact.h"
#include "configuration/configuration-file.h"
#include "contacts/contact-set.h"
#include "contacts/contact.h"
#include "core/core.h"
#include "gui/widgets/chat-widget/chat-widget-actions.h"
#include "gui/widgets/chat-widget/chat-widget-container.h"
#include "gui/widgets/chat-widget/chat-widget-factory.h"
#include "gui/widgets/chat-widget/chat-widget-repository.h"
#include "gui/widgets/chat-widget/chat-widget.h"
#include "gui/windows/chat-window/chat-window-storage.h"
#include "gui/windows/chat-window/chat-window-repository.h"
#include "gui/windows/chat-window/chat-window.h"
#include "gui/windows/kadu-window.h"
#include "icons/icons-manager.h"
#include "message/message-manager.h"
#include "message/message-render-info.h"
#include "message/unread-message-repository.h"
#include "protocols/protocol-factory.h"
#include "services/notification-service.h"
#include "activate.h"

ChatWidgetManager * ChatWidgetManager::m_instance = nullptr;

ChatWidgetManager * ChatWidgetManager::instance()
{
	if (!m_instance)
		m_instance = new ChatWidgetManager();

	return m_instance;
}

ChatWidgetManager::ChatWidgetManager() :
		m_persistedChatWindowsOpened(false)
{
	MessageRenderInfo::registerParserTags();

	connect(MessageManager::instance(), SIGNAL(messageReceived(const Message &)),
			this, SLOT(messageReceived(const Message &)));
	connect(MessageManager::instance(), SIGNAL(messageSent(const Message &)),
			this, SLOT(messageSent(const Message &)));

	m_actions = new ChatWidgetActions(this);

	configurationUpdated();
}

ChatWidgetManager::~ChatWidgetManager()
{
	MessageRenderInfo::unregisterParserTags();

	disconnect(MessageManager::instance(), nullptr, this, nullptr);

	closeAllWindows();
}

void ChatWidgetManager::setChatWidgetRepository(ChatWidgetRepository *chatWidgetRepository)
{
	m_chatWidgetRepository = chatWidgetRepository;

	if (m_chatWidgetRepository)
		connect(m_chatWidgetRepository.data(), SIGNAL(chatWidgetCreated(ChatWidget*)),
				this, SLOT(chatWidgetCreated(ChatWidget*)));
}

void ChatWidgetManager::setChatWindowRepository(ChatWindowRepository *chatWindowRepository)
{
	m_chatWindowRepository = chatWindowRepository;
	openPersistedChatWindows();
}

void ChatWidgetManager::setChatWindowStorage(ChatWindowStorage *chatWindowPeristence)
{
	m_chatWindowStorage = chatWindowPeristence;
	openPersistedChatWindows();
}

void ChatWidgetManager::openPersistedChatWindows()
{
	if (m_persistedChatWindowsOpened || !m_chatWindowStorage || !m_chatWindowRepository)
		return;

	m_persistedChatWindowsOpened = true;

	m_chatWindowStorage.data()->ensureLoaded();
	auto chats = m_chatWindowStorage.data()->loadedChats();
	foreach (const auto &chat, chats)
		byChat(chat, true);
}

void ChatWidgetManager::closeAllWindows()
{
	if (m_chatWindowStorage)
		m_chatWindowStorage.data()->ensureStored();

	if (!m_chatWidgetRepository)
		return;

	auto chatWindows = QVector<ChatWindow *>(m_chatWidgetRepository.data()->widgets().size());
	std::transform(m_chatWidgetRepository.data()->widgets().begin(), m_chatWidgetRepository.data()->widgets().end(),
		chatWindows.begin(), [](ChatWidget *chatWidget){ return qobject_cast<ChatWindow *>(chatWidget->window()); });
	qDeleteAll(chatWindows);
}

ChatWidget * ChatWidgetManager::byChat(const Chat &chat, const bool create)
{
	if (!chat || !m_chatWidgetRepository)
		return nullptr;

	if (m_chatWidgetRepository.data()->hasWidgetForChat(chat) || create)
		return m_chatWidgetRepository.data()->widgetForChat(chat);

	return nullptr;
}

void ChatWidgetManager::chatWidgetCreated(ChatWidget *chatWidget)
{
	// We need to append unread messages before chat widget container could mark them as read.
	auto messages = loadUnreadMessages(chatWidget->chat());
	chatWidget->appendMessages(messages);

	bool handled = false;
	emit handleNewChatWidget(chatWidget, handled);
	if (!handled && m_chatWindowRepository)
	{
		auto chatWindow = m_chatWindowRepository.data()->windowForChatWidget(chatWidget);
		if (chatWindow)
		{
			chatWidget->setContainer(chatWindow);
			chatWindow->show();
		}
	}
}

QVector<Message> ChatWidgetManager::loadUnreadMessages(const Chat &chat)
{
	auto buddyChat = BuddyChatManager::instance()->buddyChat(chat);
	auto unreadChat = buddyChat ? buddyChat : chat;
	auto unreadMessages = Core::instance()->unreadMessageRepository()->unreadMessagesForChat(unreadChat);
	Core::instance()->unreadMessageRepository()->markMessagesAsRead(unreadMessages);
	return unreadMessages;
}

void ChatWidgetManager::closeChat(const Chat &chat)
{
	ChatWidget * const chatWidget = byChat(chat, false);
	if (chatWidget && chatWidget->container())
		chatWidget->container()->closeChatWidget(chatWidget);
}

void ChatWidgetManager::closeAllChats(const Buddy &buddy)
{
	foreach (const Contact &contact, buddy.contacts())
	{
		const Chat &chat = ChatTypeContact::findChat(contact, ActionReturnNull);
		if (chat)
			closeChat(chat);
	}
}

void ChatWidgetManager::configurationUpdated()
{
	m_openChatOnMessage = config_file.readBoolEntry("Chat", "OpenChatOnMessage");
	m_openChatOnMessageOnlyWhenOnline = config_file.readBoolEntry("Chat", "OpenChatOnMessageWhenOnline");
}

bool ChatWidgetManager::shouldOpenChatWidget(const Message &message)
{
	if (!m_openChatOnMessage)
		return false;

	if ((m_openChatOnMessage || m_openChatOnMessageOnlyWhenOnline) && Core::instance()->notificationService()->silentMode())
		return false;

	const Protocol * const handler = message.messageChat().chatAccount().protocolHandler();
	if (!handler)
		return false;

	if (!m_openChatOnMessageOnlyWhenOnline)
		return true;

	return StatusTypeGroupOnline == handler->status().group();
}

void ChatWidgetManager::messageReceived(const Message &message)
{
	const Chat &chat = message.messageChat();
	ChatWidget *alreadyOpenedChatWidget = byChat(chat, false);

	if (alreadyOpenedChatWidget)
	{
		alreadyOpenedChatWidget->appendMessage(message);
		return;
	}

	if (!shouldOpenChatWidget(message))
	{
		qApp->alert(Core::instance()->kaduWindow());
		return;
	}

	byChat(chat, true);
}

void ChatWidgetManager::messageSent(const Message &message)
{
	const Chat &chat = message.messageChat();
	ChatWidget * const chatWidget = byChat(chat, false);
	if (!chatWidget)
		return;

	chatWidget->appendMessage(message);
}

#include "moc_chat-widget-manager.cpp"