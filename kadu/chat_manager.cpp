/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qcursor.h>
#include <qbitmap.h>
#include <qtooltip.h>

#include "action.h"
#include "chat_manager_slots.h"
#include "config_dialog.h"
#include "debug.h"
#include "chat_manager.h"
#include "history.h"
#include "icons_manager.h"
#include "kadu.h"
#include "kadu_splitter.h"
#include "misc.h"
#include "pending_msgs.h"
#include "search.h"
#include "userbox.h"
#include "ignore.h"

// TODO: make new icons for iconset instead of tricks like that
static QPixmap string_to_pixmap(const QString& str, const QFont& font)
{
	QSize size = QFontMetrics(font).size(0, str);
	QPixmap pixmap(size);
	pixmap.fill(kadu->palette().color(QPalette::Active, QColorGroup::Button));
	QPainter* painter = new QPainter();
	painter->begin(&pixmap);
	painter->setPen(kadu->palette().color(QPalette::Active, QColorGroup::ButtonText));
	painter->setFont(font);
	painter->drawText(QRect(QPoint(0, 0), size), 0, str);
	painter->end();
	pixmap.setMask(pixmap.createHeuristicMask());
	delete painter;
	return pixmap;
}

ChatManager::ChatManager(QObject* parent, const char* name)
	: QObject(parent, name), Chats(), addons(), refreshTitlesTimer()
{
	kdebugf();

	Action* auto_send_action = new Action(icons_manager->loadIcon("AutoSendMessage"),
		tr("%1 sends message").arg(config_file.readEntry("ShortCuts", "chat_newline")),
		"autoSendAction");
	auto_send_action->setToggleAction(true);
	auto_send_action->setDockAreaGroupRestriction("chatDockAreaGroup");
	connect(auto_send_action, SIGNAL(activated(const UserGroup*, const QWidget*, bool)),
		this, SLOT(autoSendActionActivated(const UserGroup*, const QWidget*, bool)));
	connect(auto_send_action, SIGNAL(addedToToolbar(ToolButton*, ToolBar*,
			const UserListElements&)),
		this, SLOT(autoSendActionAddedToToolbar(ToolButton*, ToolBar*,
			const UserListElements&)));
	KaduActions.insert("autoSendAction", auto_send_action);

	Action* scroll_lock_action = new Action(icons_manager->loadIcon("ScrollLock"),
		tr("Blocks scrolling"), "scrollLockAction");
	scroll_lock_action->setToggleAction(true);
	scroll_lock_action->setDockAreaGroupRestriction("chatDockAreaGroup");
	connect(scroll_lock_action, SIGNAL(activated(const UserGroup*, const QWidget*, bool)),
		this, SLOT(scrollLockActionActivated(const UserGroup*, const QWidget*, bool)));
	KaduActions.insert("scrollLockAction", scroll_lock_action);

	Action* clear_action = new Action(icons_manager->loadIcon("ClearChat"),
		tr("Clear messages in chat window"), "clearChatAction");
	clear_action->setDockAreaGroupRestriction("chatDockAreaGroup");
	connect(clear_action, SIGNAL(activated(const UserGroup*, const QWidget*, bool)),
		this, SLOT(clearActionActivated(const UserGroup*)));
	KaduActions.insert("clearChatAction", clear_action);

	Action* history_action = new Action(icons_manager->loadIcon("History"),
		tr("Show history"), "showHistoryAction");
	connect(history_action, SIGNAL(activated(const UserGroup*, const QWidget*, bool)),
		this, SLOT(historyActionActivated(const UserGroup*)));
	KaduActions.insert("showHistoryAction", history_action);

	Action* insert_emot_action = new Action(icons_manager->loadIcon("ChooseEmoticon"),
		tr("Insert emoticon"), "insertEmoticonAction");
	insert_emot_action->setDockAreaGroupRestriction("chatDockAreaGroup");
	connect(insert_emot_action, SIGNAL(activated(const UserGroup*, const QWidget*, bool)),
		this, SLOT(insertEmoticonActionActivated(const UserGroup*, const QWidget*)));
	connect(insert_emot_action, SIGNAL(addedToToolbar(ToolButton*, ToolBar*,
			const UserListElements&)),
		this, SLOT(insertEmoticonActionAddedToToolbar(ToolButton*, ToolBar*,
			const UserListElements&)));
	KaduActions.insert("insertEmoticonAction", insert_emot_action);

	Action* whois_action = new Action(icons_manager->loadIcon("LookupUserInfo"),
		tr("Search this user in directory"), "whoisAction");
	whois_action->setDockAreaGroupRestriction("chatDockAreaGroup");
	connect(whois_action, SIGNAL(activated(const UserGroup*, const QWidget*, bool)),
		this, SLOT(whoisActionActivated(const UserGroup*)));
	KaduActions.insert("whoisAction", whois_action);

	Action* insert_img_action = new Action(icons_manager->loadIcon("ChooseImage"),
		tr("Insert image"), "insertImageAction");
	insert_img_action->setDockAreaGroupRestriction("chatDockAreaGroup");
	connect(insert_img_action, SIGNAL(activated(const UserGroup*, const QWidget*, bool)),
		this, SLOT(insertImageActionActivated(const UserGroup*)));
	KaduActions.insert("insertImageAction", insert_img_action);

	Action* ignore_user_action = new Action(icons_manager->loadIcon("ManageIgnored"),
		tr("Ignore user"), "ignoreUserAction");
	connect(ignore_user_action, SIGNAL(activated(const UserGroup*, const QWidget*, bool)),
		this, SLOT(ignoreUserActionActivated(const UserGroup*)));
	KaduActions.insert("ignoreUserAction", ignore_user_action);
	
	Action* block_user_action = new Action(icons_manager->loadIcon("Blocking"),
		tr("Block user"), "blockUserAction");
	connect(block_user_action, SIGNAL(activated(const UserGroup*, const QWidget*, bool)),
		this, SLOT(blockUserActionActivated(const UserGroup*)));
	KaduActions.insert("blockUserAction", block_user_action);
	
	QFont font;

	font.setBold(true);
	Action* bold_action = new Action(string_to_pixmap("B", font),
		tr("Bold"), "boldAction");
	bold_action->setToggleAction(true);
	bold_action->setDockAreaGroupRestriction("chatDockAreaGroup");
	connect(bold_action, SIGNAL(activated(const UserGroup*, const QWidget*, bool)),
		this, SLOT(boldActionActivated(const UserGroup*, const QWidget*, bool)));
	KaduActions.insert("boldAction", bold_action);

	font.setBold(false);
	font.setItalic(true);
	Action* italic_action = new Action(string_to_pixmap("I", font),
		tr("Italic"), "italicAction");
	italic_action->setToggleAction(true);
	italic_action->setDockAreaGroupRestriction("chatDockAreaGroup");
	connect(italic_action, SIGNAL(activated(const UserGroup*, const QWidget*, bool)),
		this, SLOT(italicActionActivated(const UserGroup*, const QWidget*, bool)));
	KaduActions.insert("italicAction", italic_action);

	font.setItalic(false);
	font.setUnderline(true);
	Action* underline_action = new Action(string_to_pixmap("U", font),
		tr("Underline"), "underlineAction");
	underline_action->setToggleAction(true);
	underline_action->setDockAreaGroupRestriction("chatDockAreaGroup");
	connect(underline_action, SIGNAL(activated(const UserGroup*, const QWidget*, bool)),
		this, SLOT(underlineActionActivated(const UserGroup*, const QWidget*, bool)));
	KaduActions.insert("underlineAction", underline_action);

	QPixmap p(12, 12);
	p.fill(Qt::black);
	Action* color_action = new Action(p,
		tr("Change color"), "colorAction");
	color_action->setDockAreaGroupRestriction("chatDockAreaGroup");
	connect(color_action, SIGNAL(activated(const UserGroup*, const QWidget*, bool)),
		this, SLOT(colorActionActivated(const UserGroup*, const QWidget*)));
	KaduActions.insert("colorAction", color_action);

	Action* send_action = new Action(icons_manager->loadIcon("SendMessage"),
		tr("&Send"), "sendAction");
	send_action->setDockAreaGroupRestriction("chatDockAreaGroup");
	connect(send_action, SIGNAL(activated(const UserGroup*, const QWidget*, bool)),
		this, SLOT(sendActionActivated(const UserGroup*)));
	KaduActions.insert("sendAction", send_action);

	Action* chat_action = new Action(icons_manager->loadIcon("OpenChat"),
		tr("&Chat"), "chatAction");
	connect(chat_action, SIGNAL(activated(const UserGroup*, const QWidget*, bool)),
		this, SLOT(chatActionActivated(const UserGroup*)));
	KaduActions.insert("chatAction", chat_action);

	KaduActions.addDefaultToolbarAction("Chat toolbar 1", "autoSendAction");
	KaduActions.addDefaultToolbarAction("Chat toolbar 1", "scrollLockAction");
	KaduActions.addDefaultToolbarAction("Chat toolbar 1", "clearChatAction");
	KaduActions.addDefaultToolbarAction("Chat toolbar 1", "showHistoryAction");
	KaduActions.addDefaultToolbarAction("Chat toolbar 1", "insertEmoticonAction");
	KaduActions.addDefaultToolbarAction("Chat toolbar 1", "whoisAction");
	KaduActions.addDefaultToolbarAction("Chat toolbar 1", "insertImageAction");

	KaduActions.addDefaultToolbarAction("Chat toolbar 2", "boldAction");
	KaduActions.addDefaultToolbarAction("Chat toolbar 2", "italicAction");
	KaduActions.addDefaultToolbarAction("Chat toolbar 2", "underlineAction");
	KaduActions.addDefaultToolbarAction("Chat toolbar 2", "colorAction");

	KaduActions.addDefaultToolbarAction("Chat toolbar 3", "sendAction", -1, true);

	KaduActions.addDefaultToolbarAction("Search toolbar", "chatAction", -1, true);

	if (config_file.readBoolEntry("Chat", "RememberPosition"))
	{
		userlist->addPerContactNonProtocolConfigEntry("chat_geometry", "ChatGeometry");
		userlist->addPerContactNonProtocolConfigEntry("chat_vertical_sizes", "VerticalSizes");
	}
	connect(&refreshTitlesTimer, SIGNAL(timeout()), this, SLOT(refreshTitles()));
	connect(userlist, SIGNAL(usersStatusChanged(QString)), this, SLOT(refreshTitlesLater()));

	kdebugf2();
}

void ChatManager::closeAllWindows()
{
	kdebugf();
	while (!Chats.empty())
	{
		Chat *chat=Chats.first();
		delete chat;
	}
	kdebugf2();
}

ChatManager::~ChatManager()
{
	kdebugf();
	disconnect(&refreshTitlesTimer, SIGNAL(timeout()), this, SLOT(refreshTitles()));
	disconnect(userlist, SIGNAL(usersStatusChanged(QString)), this, SLOT(refreshTitlesLater()));
	closeAllWindows();
	kdebugf2();
}

void ChatManager::autoSendActionAddedToToolbar(ToolButton* button, ToolBar* /*toolbar*/,
	const UserListElements& users)
{
	kdebugf();
	button->setOn(findChat(users)->autoSend());
	kdebugf2();
}

void ChatManager::autoSendActionActivated(const UserGroup* users, const QWidget* /*source*/, bool is_on)
{
	kdebugf();
	findChat(users)->setAutoSend(is_on);
	kdebugf2();
}

void ChatManager::scrollLockActionActivated(const UserGroup* users, const QWidget* /*source*/, bool is_on)
{
	kdebugf();
	findChat(users)->setScrollLocked(is_on);
	kdebugf2();
}

void ChatManager::clearActionActivated(const UserGroup* users)
{
	kdebugf();
	findChat(users)->clearChatWindow();
	kdebugf2();
}

void ChatManager::historyActionActivated(const UserGroup* users)
{
	kdebugf();
	UinsList uins;
	CONST_FOREACH(user, *users)
		uins.append((*user).ID("Gadu").toUInt());
	//TODO: throw out UinsList as soon as possible!
	(new History(uins))->show();
	kdebugf2();
}

void ChatManager::boldActionActivated(const UserGroup* users, const QWidget* /*source*/, bool is_on)
{
	kdebugf();
	findChat(users)->edit()->setBold(is_on);
	kdebugf2();
}

void ChatManager::italicActionActivated(const UserGroup* users, const QWidget* /*source*/, bool is_on)
{
	kdebugf();
	findChat(users)->edit()->setItalic(is_on);
	kdebugf2();
}

void ChatManager::underlineActionActivated(const UserGroup* users, const QWidget* /*source*/, bool is_on)
{
	kdebugf();
	findChat(users)->edit()->setUnderline(is_on);
	kdebugf2();
}

void ChatManager::colorActionActivated(const UserGroup* users, const QWidget* source)
{
	kdebugf();
	findChat(users)->changeColor(source);
	kdebugf2();
}

void ChatManager::insertEmoticonActionActivated(const UserGroup* users, const QWidget* source)
{
	kdebugf();
	findChat(users)->openEmoticonSelector(source);
	kdebugf2();
}

void ChatManager::insertEmoticonActionAddedToToolbar(ToolButton* button, ToolBar* /*toolbar*/,
	const UserListElements& /*users*/)
{
	kdebugf();
	if((EmoticonsStyle)config_file.readNumEntry("Chat","EmoticonsStyle") == EMOTS_NONE)
	{
		QToolTip::remove(button);
		QToolTip::add(button, tr("Insert emoticon - enable in configuration"));
		button->setEnabled(false);
	}
	kdebugf2();
}

void ChatManager::whoisActionActivated(const UserGroup* users)
{
	kdebugf();
	if (users->count() == 0)
	{
		(new SearchDialog())->show();
	}
	else
	{
		UserListElement user = *users->constBegin();
		if (user.usesProtocol("Gadu"))
		{
			UinType uin = user.ID("Gadu").toUInt();
			SearchDialog *sd = new SearchDialog(0, QString("SearchDialog:%1").arg(uin).local8Bit().data(), uin);
			sd->show();
			sd->firstSearch();
		}
	}
	kdebugf2();
}

void ChatManager::ignoreUserActionActivated(const UserGroup* users)
{
	kdebugf();
	if (users->count() > 0)
	{
		UserListElements u = users->toUserListElements();
		if (isIgnored(u))
			delIgnored(u);
		else
		{
			addIgnored(u);
			Chat *c = findChat(users);
			if (c)
				c->close();
		}
		writeIgnored();
	}
	kdebugf2();
}

void ChatManager::blockUserActionActivated(const UserGroup* users)
{
	kdebugf();
	if (users->count() > 0)
	{
		FOREACH(user, (*users))
		{
			if ((*user).usesProtocol("Gadu"))
				(*user).setProtocolData("Gadu", "Blocking", !((*user).protocolData("Gadu", "Blocking").toBool()));
		}
		userlist->writeToConfig();
	}
	kdebugf2();
}

void ChatManager::insertImageActionActivated(const UserGroup* users)
{
	kdebugf();
	findChat(users)->insertImage();
	kdebugf2();
}

void ChatManager::sendActionActivated(const UserGroup* users)
{
	kdebugf();
	Chat* chat = findChat(users);
	if (chat->waitingForACK())
		chat->cancelMessage();
	else
		chat->sendMessage();
	kdebugf2();
}

void ChatManager::chatActionActivated(const UserGroup* users)
{
	kdebugf();
	if (users->count() > 0)
	{
		bool ContainsBad = false;
		QString MyGGUIN = QString::number(config_file.readNumEntry("General", "UIN"));
	
		CONST_FOREACH(user, (*users))
		{
			if (!(*user).usesProtocol("Gadu") || (*user).ID("Gadu") == MyGGUIN)
			{
				ContainsBad = true;
				break;
			}
		}
	
		if (!ContainsBad)
			openChat("Gadu", users->toUserListElements(), 0);
	}
	kdebugf2();
}

const ChatList& ChatManager::chats() const
{
	return Chats;
}

int ChatManager::registerChat(Chat* chat)
{
	Chats.append(chat);
	return Chats.count() - 1;
}

void ChatManager::unregisterChat(Chat* chat)
{
	kdebugf();
	FOREACH(curChat, Chats)
		if (*curChat == chat)
		{
			const UserGroup *users = chat->users();
			setChatProperty(users, "Geometry", QRect(chat->pos().x(), chat->pos().y(), chat->size().width(), chat->size().height()));
			QValueList<int> sizes = chat->vertSplit->sizes();
			setChatProperty(users, "VerticalSizes", toVariantList(sizes));
			if (users->count() == 1)
			{
				(*users->begin()).setData("ChatGeometry", QString("%1,%2,%3,%4").arg(chat->pos().x()).arg(chat->pos().y()).arg(chat->size().width()).arg(chat->size().height()));
				(*users->begin()).setData("VerticalSizes", QString("%1,%2").arg(sizes[0]).arg(sizes[1]));
			}
			if (chat->horizSplit)
				setChatProperty(users, "HorizontalSizes", toVariantList(chat->horizSplit->sizes()));

			emit chatDestroying(users);
			Chats.remove(curChat);
			emit chatDestroyed(users);
			kdebugf2();
			return;
		}
	kdebugmf(KDEBUG_FUNCTION_END|KDEBUG_WARNING, "NOT found\n");
}

void ChatManager::refreshTitlesLater()
{
	refreshTitlesTimer.start(0, true);
}

void ChatManager::refreshTitles()
{
	kdebugf();
	CONST_FOREACH(chat, Chats)
		(*chat)->refreshTitle();
	kdebugf2();
}

void ChatManager::refreshTitlesForUser(UserListElement user)
{
	kdebugf();
	CONST_FOREACH(chat, Chats)
		if ((*chat)->users()->contains(user))
			(*chat)->refreshTitle();
	kdebugf2();
}

void ChatManager::changeAppearance()
{
	kdebugf();
	CONST_FOREACH(chat, Chats)
		(*chat)->changeAppearance();
	kdebugf2();
}

Chat* ChatManager::findChat(const UserGroup *group) const
{
	CONST_FOREACH(chat, Chats)
		if ((*chat)->users() == group)
			return *chat;
	kdebugmf(KDEBUG_WARNING, "no such chat\n");
	return NULL;
}

Chat* ChatManager::findChat(UserListElements users) const
{
	CONST_FOREACH(chat, Chats)
		if (users.equals((*chat)->users()))
			return *chat;
	kdebugmf(KDEBUG_WARNING, "no such chat\n");
	return NULL;
}

int ChatManager::openChat(QString /*initialProtocol*/, UserListElements users, time_t time)
{
	kdebugf();
	emit chatOpen(users);
	unsigned int i = 0;
	CONST_FOREACH(chat, Chats)
	{
		if ((*chat)->users()->equals(users))
		{
#if QT_VERSION >= 0x030300
			if (qstrcmp(qVersion(), "3.3") >= 0) // dodatkowe zabezpieczenie przed idiotami używającymi opcji --force przy instalacji pakietów
				(*chat)->setWindowState((*chat)->windowState() & ~WindowMinimized);
#endif
			(*chat)->raise();
			(*chat)->setActiveWindow();
			return i;
		}
		++i;
	}
	QStringList userNames;
	CONST_FOREACH(user, users)
		userNames.append((*user).altNick());
	userNames.sort();

	Chat* chat = new Chat(users, 0, QString("chat:%1").arg(userNames.join(",")).local8Bit().data());
	chat->refreshTitle();
	connect(chat, SIGNAL(messageSentAndConfirmed(UserListElements, const QString&)),
		this, SIGNAL(messageSentAndConfirmed(UserListElements, const QString&)));

	const UserGroup *group = chat->users();
	QRect geometry = getChatProperty(group, "Geometry").toRect();
	if (geometry.isEmpty() && group->count() == 1)
	{
		QString geo_str = (*(group->constBegin())).data("ChatGeometry").toString();
		if (!geo_str.isEmpty())
		{
			bool ok[4];
			QStringList s = QStringList::split(",", geo_str);
			geometry.setX(s[0].toInt(ok));
			geometry.setY(s[1].toInt(ok + 1));
			geometry.setWidth(s[2].toInt(ok + 2));
			geometry.setHeight(s[3].toInt(ok + 3));
			if (int(ok[0]) + ok [1] + ok [2] + ok [3] != 4)
				geometry = QRect();
		}
	}
	if (geometry.isEmpty())
	{
		QPoint pos = QCursor::pos();
		int x,y,width,height;
		QDesktopWidget *desk=qApp->desktop();
		x=pos.x()+50;
		y=pos.y()+50;
		height=400;

		if (group->count() > 1)
			width=550;
		else
			width=400;
		if (x+width>desk->width())
			x=desk->width()-width-50;
		if (y+height>desk->height())
			y=desk->height()-height-50;
		if (x<50) x=50;
		if (y<50) y=50;
		geometry.setX(x);
		geometry.setY(y);
		geometry.setWidth(width);
		geometry.setHeight(height);
	}
	chat->setGeometry(geometry);

	QValueList<int> vertSizes = toIntList(getChatProperty(group, "VerticalSizes").toList());
	if (vertSizes.empty() && group->count() == 1)
	{
		QString vert_sz_str = (*(group->constBegin())).data("VerticalSizes").toString();
		if (!vert_sz_str.isEmpty())
		{
			bool ok[2];
			QStringList s = QStringList::split(",", vert_sz_str);
			vertSizes.append(s[0].toInt(ok));
			vertSizes.append(s[1].toInt(ok + 1));
			if (!(ok[0] && ok[1]))
				vertSizes.clear();
		}
	}
	
	if (vertSizes.empty())
	{
		int h = chat->height() / 3;
		vertSizes.append(h * 2);
		vertSizes.append(h);
	}
	chat->vertSplit->setSizes(vertSizes);

	if (chat->horizSplit)
	{
		QValueList<int> horizSizes = toIntList(getChatProperty(group, "HorizontalSizes").toList());
		if (!horizSizes.empty())
			chat->horizSplit->setSizes(horizSizes);
	}

	chat->show();
	chat->writeMessagesFromHistory(users, time);
	emit chatCreated(group);
	kdebugf2();
	return Chats.count() - 1;
}

int ChatManager::openPendingMsg(int index, ChatMessage &msg)
{
	kdebugf();
	// TODO: check if index does not exceed boundaries
	PendingMsgs::Element p = pending[index];
	// opening chat (if it does not exist)
	int k = openChat("Gadu", p.users, p.time);
	// appending new message

	QDateTime date;
	date.setTime_t(p.time);

	msg = ChatMessage(p.users[0].altNick(), p.msg, false, QDateTime::currentDateTime(), date);
	Chats[k]->formatMessage(msg);

	// remove message from pending
	pending.deleteMsg(index);
	kdebugf2();
	// return index of opened chat window
	return k;
}

void ChatManager::deletePendingMsgs(UserListElements users)
{
	kdebugf();

	for (int i = 0; i < pending.count(); ++i)
		if (pending[i].users.equals(users))
		{
			pending.deleteMsg(i);
			--i;
		}

	UserBox::refreshAllLater();
	kdebugf2();
}

void ChatManager::openPendingMsgs(UserListElements users)
{
	kdebugf();
	PendingMsgs::Element elem;
	int k;
	bool stop = false;

	QValueList<ChatMessage *> messages;
	for (int i = 0; i < pending.count(); ++i)
	{
		elem = pending[i];
		if (elem.users.equals(users))
			if ((elem.msgclass & GG_CLASS_CHAT) == GG_CLASS_CHAT
				|| (elem.msgclass & GG_CLASS_MSG) == GG_CLASS_MSG
				|| !elem.msgclass)
			{
				ChatMessage *msg = new ChatMessage(QString::null);
				k = openPendingMsg(i, *msg);
				messages.append(msg);

				--i;
				users = elem.users;
				stop = true;
			}
	}
	if (stop)
	{
		Chats[k]->scrollMessages(messages);
		UserBox::refreshAllLater();
	}
	else
		k = openChat("Gadu", users, 0);
	kdebugf2();
}

void ChatManager::openPendingMsgs()
{
	kdebugf();
	UserListElements users;
	int i, k = -1;
	PendingMsgs::Element elem;
	bool stop = false;
	QValueList<ChatMessage *> messages;

	for(i = 0; i < pending.count(); ++i)
	{
		elem = pending[i];
		if (users.isEmpty() || elem.users.equals(users))
			if ((elem.msgclass & GG_CLASS_CHAT) == GG_CLASS_CHAT
				|| (elem.msgclass & GG_CLASS_MSG) == GG_CLASS_MSG
				|| (!elem.msgclass))
			{
				if (users.isEmpty())
					users = elem.users;

				ChatMessage *msg = new ChatMessage(QString::null);
				k = openPendingMsg(i, *msg);
				messages.append(msg);

				--i;
				stop = true;
			}
	}
	if (stop)
	{
		kdebugmf(KDEBUG_INFO, "stopped\n");
		Chats[k]->scrollMessages(messages);
		UserBox::refreshAllLater();
	}
	kdebugf2();
}

void ChatManager::sendMessage(UserListElement user, UserListElements selected_users)
{
	kdebugf();
	QString tmp;
	int i, k = -1;
	bool stop = false;
	PendingMsgs::Element elem;
	UserListElements users;
	QValueList<ChatMessage *> messages;

	for (i = 0; i < pending.count(); ++i)
	{
		elem = pending[i];
		if ((users.isEmpty() && elem.users.contains(user)) || (!users.isEmpty() && elem.users.equals(users)))
			if ((elem.msgclass & GG_CLASS_CHAT) == GG_CLASS_CHAT
				|| (elem.msgclass & GG_CLASS_MSG) == GG_CLASS_MSG
				|| !elem.msgclass)
			{
				if (users.isEmpty())
					users = elem.users;

				ChatMessage *msg = new ChatMessage(QString::null);
				k = openPendingMsg(i, *msg);
				messages.append(msg);

				--i;
				stop = true;
			}
	}
	if (stop)
	{
		Chats[k]->scrollMessages(messages);
		UserBox::refreshAllLater();
	}
	else
		k = openChat("Gadu", selected_users, 0);
	kdebugf2();
}

void ChatManager::chatMsgReceived(Protocol *protocol, UserListElements senders, const QString& msg, time_t time, bool& grab)
{
	Chat *chat = findChat(senders);
	if (chat != NULL)
	{
		chat->newMessage(protocol->protocolID(), senders, msg, time);
		chat->alertNewMessage();
		grab = true;
	}
}

QVariant& ChatManager::getChatProperty(const UserGroup *group, const QString &name)
{
	kdebugf();
	FOREACH(addon, addons)
		if (group->equals((*addon).users))
		{
			kdebugf2();
			return (*addon).map[name];
		}
	ChatInfo info;
	info.users = group->toUserListElements();
	info.map[name] = QVariant();
	addons.push_front(info);
	kdebugmf(KDEBUG_FUNCTION_END, "end: %s NOT found\n", name.local8Bit().data());
	return addons[0].map[name];
}

void ChatManager::setChatProperty(const UserGroup *group, const QString &name, const QVariant &value)
{
	kdebugf();
	FOREACH(addon, addons)
		if (group->equals((*addon).users))
		{
			(*addon).map[name] = value;
			kdebugf2();
			return;
		}
	ChatInfo info;
	info.users = group->toUserListElements();
	info.map[name] = value;
	addons.push_front(info);
	kdebugf2();
}

void ChatManager::closeModule()
{
	kdebugf();
	disconnect(gadu, SIGNAL(chatMsgReceived1(Protocol *, UserListElements, const QString&, time_t, bool&)),
		chat_manager, SLOT(chatMsgReceived(Protocol *, UserListElements, const QString&, time_t, bool&)));

	ConfigDialog::disconnectSlot("Look", "Font in chat window", SIGNAL(changed(const char *, const QFont&)), chatslots, SLOT(chooseFont(const char *, const QFont&)), "chat_font_box");
	ConfigDialog::disconnectSlot("Look", "Select chat style", SIGNAL(activated(const QString&)), chatslots, SLOT(onChatThemeChanged(const QString&)));

	ConfigDialog::disconnectSlot("Chat", "Emoticons:", SIGNAL(activated(int)), chatslots, SLOT(chooseEmoticonsStyle(int)));
	ConfigDialog::disconnectSlot("Chat", "Automatically prune chat messages", SIGNAL(toggled(bool)), chatslots, SLOT(onPruneChat(bool)));
	ConfigDialog::disconnectSlot("Chat", "Automatically fold links", SIGNAL(toggled(bool)), chatslots, SLOT(onFoldLink(bool)));
	ConfigDialog::disconnectSlot("Chat", "Block window close on new message", SIGNAL(toggled(bool)), chatslots, SLOT(onBlockClose(bool)));

	ConfigDialog::disconnectSlot("Look", "Remove chat header repetitions", SIGNAL(toggled(bool)), chatslots, SLOT(onRemoveHeaders(bool)));

	ConfigDialog::disconnectSlot("Look", "Your background color", SIGNAL(changed(const char *, const QColor&)),
		chatslots, SLOT(chooseColor(const char *, const QColor&)), "own_bg_color");
	ConfigDialog::disconnectSlot("Look", "User background color", SIGNAL(changed(const char *, const QColor&)),
		chatslots, SLOT(chooseColor(const char *, const QColor&)), "his_bg_color");
	ConfigDialog::disconnectSlot("Look", "Your font color", SIGNAL(changed(const char *, const QColor&)),
		chatslots, SLOT(chooseColor(const char *, const QColor&)), "own_font_color");
	ConfigDialog::disconnectSlot("Look", "User font color", SIGNAL(changed(const char *, const QColor&)),
		chatslots, SLOT(chooseColor(const char *, const QColor&)), "his_font_color");

	ConfigDialog::unregisterSlotOnCreateTab("Chat", chatslots, SLOT(onCreateTabChat()));
	ConfigDialog::unregisterSlotOnCreateTab("Look", chatslots, SLOT(onCreateTabLook()));
	ConfigDialog::unregisterSlotOnApplyTab("Chat", chatslots, SLOT(onApplyTabChat()));
	ConfigDialog::unregisterSlotOnApplyTab("Look", chatslots, SLOT(onApplyTabLook()));

			ConfigDialog::removeControl("Look", "syntax:");
			ConfigDialog::removeControl("Look", "Conference window title prefix:");
		ConfigDialog::removeControl("Look", "conference");
		ConfigDialog::removeControl("Look", "Chat window title syntax:");
	ConfigDialog::removeControl("Look", "Other");

		ConfigDialog::removeControl("Look", "Interval between header removal:");
		ConfigDialog::removeControl("Look", "Chat header separators height:");
		ConfigDialog::removeControl("Look", "Remove chat header repetitions");
	ConfigDialog::removeControl("Look", "Headers");

		ConfigDialog::removeControl("Look", "Full chat style:");
		ConfigDialog::removeControl("Look", "Select chat style");
	ConfigDialog::removeControl("Look", "Style");

				ConfigDialog::removeControl("Look", "<b>Other party</b> 00:00:02", "chat_other");
				ConfigDialog::removeControl("Look", "<b>Me</b> 00:00:00", "chat_me");
			ConfigDialog::removeControl("Look", "chat_prvw");
		ConfigDialog::removeControl("Look", "Chat preview");
	ConfigDialog::removeControl("Look", "Previews");

		ConfigDialog::removeControl("Look", "Font in chat window", "chat_font_box");
//	ConfigDialog::removeControl("Look", "Fonts");

			ConfigDialog::removeControl("Look", "User nick color", "his_nick_color");
			ConfigDialog::removeControl("Look", "Your nick color", "own_nick_color");
			ConfigDialog::removeControl("Look", "User font color", "his_font_color");
			ConfigDialog::removeControl("Look", "Your font color", "own_font_color");
			ConfigDialog::removeControl("Look", "User background color", "his_bg_color");
			ConfigDialog::removeControl("Look", "Your background color", "own_bg_color");
			ConfigDialog::removeControl("Look", "Text edit background color", "text_bg_color");
			ConfigDialog::removeControl("Look", "Chat window background color", "bg_color");
		ConfigDialog::removeControl("Look", "Chat window");
//	ConfigDialog::removeControl("Look", "Colors");

	ConfigDialog::removeControl("Look", "varOpts2");

	ConfigDialog::removeControl("Chat", "Automatically fold links longer than");
	ConfigDialog::removeControl("Chat", "Link folding");
	ConfigDialog::removeControl("Chat", "Automatically fold links");

	ConfigDialog::removeControl("Chat", "Reduce the number of visible messages to");
	ConfigDialog::removeControl("Chat", "Message pruning");
	ConfigDialog::removeControl("Chat", "Automatically prune chat messages");

	ConfigDialog::removeControl("Chat", "Max time to block window close");
	ConfigDialog::removeControl("Chat", "Remember chat windows positions");
	ConfigDialog::removeControl("Chat", "Receive images during invisibility");
	ConfigDialog::removeControl("Chat", "Block window close on new message");
	ConfigDialog::removeControl("Chat", "Ignore richtext from anonymous users");
	ConfigDialog::removeControl("Chat", "Ignore messages from anonymous users");
	ConfigDialog::removeControl("Chat", "Flash chat title on new message");
	ConfigDialog::removeControl("Chat", "\"%1\" in chat sends message by default");
	ConfigDialog::removeControl("Chat", "Message acknowledgements (wait for delivery)");
	ConfigDialog::removeControl("Chat", "Open chat window on new message");

	ConfigDialog::removeControl("Chat", "checkboxes-beginner");
	ConfigDialog::removeControl("Chat", "checkboxes-advanced");
	ConfigDialog::removeControl("Chat", "checkboxes-expert");

	ConfigDialog::removeControl("Chat", "Max image size");

	ConfigDialog::removeControl("Chat", "Custom Web browser");
	ConfigDialog::removeControl("Chat", "Browser options");
	ConfigDialog::removeControl("Chat", "Choose your browser");
	ConfigDialog::removeControl("Chat", "WWW options");

	ConfigDialog::removeControl("Chat", "Emoticons theme");

	ConfigDialog::removeControl("Chat", "Emoticons:");
	ConfigDialog::removeControl("Chat", "Emoticons");

	ConfigDialog::removeControl("ShortCuts", "Underline text:");
	ConfigDialog::removeControl("ShortCuts", "Italic text:");
	ConfigDialog::removeControl("ShortCuts", "Bold text:");
	ConfigDialog::removeControl("ShortCuts", "Close Chat:");
	ConfigDialog::removeControl("ShortCuts", "Clear Chat:");
	ConfigDialog::removeControl("ShortCuts", "New line / send message:");
//	ConfigDialog::removeControl("ShortCuts", "Define keys");

	delete chatslots;
	chatslots = 0;
	delete chat_manager;
	chat_manager = 0;
	kdebugf2();
}

void ChatManager::initModule()
{
	kdebugf();

//	ConfigDialog::addVGroupBox("ShortCuts", "ShortCuts", QT_TRANSLATE_NOOP("@default", "Define keys"));
	ConfigDialog::addHotKeyEdit("ShortCuts", "Define keys", QT_TRANSLATE_NOOP("@default", "New line / send message:"), "chat_newline", "Return");
	ConfigDialog::addHotKeyEdit("ShortCuts", "Define keys", QT_TRANSLATE_NOOP("@default", "Clear Chat:"), "chat_clear", "F9");
	ConfigDialog::addHotKeyEdit("ShortCuts", "Define keys", QT_TRANSLATE_NOOP("@default", "Close Chat:"), "chat_close", "Esc");
	ConfigDialog::addHotKeyEdit("ShortCuts", "Define keys", QT_TRANSLATE_NOOP("@default", "Bold text:"), "chat_bold", "Ctrl+B");
	ConfigDialog::addHotKeyEdit("ShortCuts", "Define keys", QT_TRANSLATE_NOOP("@default", "Italic text:"), "chat_italic", "Ctrl+I");
	ConfigDialog::addHotKeyEdit("ShortCuts", "Define keys", QT_TRANSLATE_NOOP("@default", "Underline text:"), "chat_underline", "Ctrl+U");

	ConfigDialog::addVGroupBox("Chat", "Chat", QT_TRANSLATE_NOOP("@default", "Emoticons"), 0, Beginner);
	ConfigDialog::addComboBox("Chat", "Emoticons", QT_TRANSLATE_NOOP("@default", "Emoticons:"),
			"EmoticonsStyle", toStringList(tr("None"), tr("Static"), tr("Animated")), toStringList("0", "1", "2"), "2");

	ConfigDialog::addComboBox("Chat", "Emoticons", QT_TRANSLATE_NOOP("@default", "Emoticons theme"));

	ConfigDialog::addVGroupBox("Chat", "Chat", QT_TRANSLATE_NOOP("@default", "WWW options"), 0, Beginner);
	ConfigDialog::addComboBox("Chat", "WWW options", QT_TRANSLATE_NOOP("@default", "Choose your browser"));
	ConfigDialog::addComboBox("Chat", "WWW options", QT_TRANSLATE_NOOP("@default", "Browser options"));
	ConfigDialog::addLineEdit("Chat", "WWW options", QT_TRANSLATE_NOOP("@default", "Custom Web browser"), "WebBrowser", 0, QT_TRANSLATE_NOOP("@default", "%1 - Url clicked in chat window"));

	ConfigDialog::addSpinBox("Chat", "Chat", QT_TRANSLATE_NOOP("@default", "Max image size"),
			"MaxImageSize", 0, 255, 5, 20);

	ConfigDialog::addVBox("Chat", "Chat", "checkboxes-beginner", 0, Beginner);
	ConfigDialog::addVBox("Chat", "Chat", "checkboxes-advanced", 0, Advanced);
	ConfigDialog::addVBox("Chat", "Chat", "checkboxes-expert", 0, Expert);

	ConfigDialog::addCheckBox("Chat", "checkboxes-beginner", QT_TRANSLATE_NOOP("@default", "Open chat window on new message"), "OpenChatOnMessage", false);
	ConfigDialog::addCheckBox("Chat", "checkboxes-beginner", QT_TRANSLATE_NOOP("@default", "Message acknowledgements (wait for delivery)"), "MessageAcks", true);
	ConfigDialog::addCheckBox("Chat", "checkboxes-advanced", QT_TRANSLATE_NOOP("@default", "\"%1\" in chat sends message by default"), "AutoSend", true, 0, 0, Advanced);
	ConfigDialog::addCheckBox("Chat", "checkboxes-advanced", QT_TRANSLATE_NOOP("@default", "Flash chat title on new message"), "BlinkChatTitle", true, 0, 0, Advanced);
	ConfigDialog::addCheckBox("Chat", "checkboxes-advanced", QT_TRANSLATE_NOOP("@default", "Ignore messages from anonymous users"), "IgnoreAnonymousUsers", false, 0, 0, Advanced);
	ConfigDialog::addCheckBox("Chat", "checkboxes-advanced", QT_TRANSLATE_NOOP("@default", "Ignore richtext from anonymous users"), "IgnoreAnonymousRichtext", true, 0, 0, Advanced);
	ConfigDialog::addCheckBox("Chat", "checkboxes-advanced", QT_TRANSLATE_NOOP("@default", "Block window close on new message"), "ChatCloseTimer", false, 0, 0, Advanced);
	ConfigDialog::addCheckBox("Chat", "checkboxes-expert", QT_TRANSLATE_NOOP("@default", "Receive images during invisibility"), "ReceiveImagesDuringInvisibility", true, 0, 0, Expert);
	ConfigDialog::addCheckBox("Chat", "checkboxes-expert", QT_TRANSLATE_NOOP("@default", "Remember chat windows positions"), "RememberPosition", true, 0, 0, Expert);
	ConfigDialog::addSpinBox("Chat", "Chat", QT_TRANSLATE_NOOP("@default", "Max time to block window close"),
			"ChatCloseTimerPeriod", 1, 5, 1, 2, 0, 0, Expert);

	ConfigDialog::addCheckBox("Chat", "Chat", QT_TRANSLATE_NOOP("@default", "Automatically prune chat messages"), "ChatPrune", true, 0, 0, Advanced);
	ConfigDialog::addHGroupBox("Chat", "Chat", QT_TRANSLATE_NOOP("@default", "Message pruning"), 0, Advanced);
	ConfigDialog::addSpinBox("Chat", "Message pruning", QT_TRANSLATE_NOOP("@default", "Reduce the number of visible messages to"), "ChatPruneLen", 1,255,1,20);

	ConfigDialog::addCheckBox("Chat", "Chat", QT_TRANSLATE_NOOP("@default", "Automatically fold links"), "FoldLink", false, 0, 0, Advanced);
	ConfigDialog::addHGroupBox("Chat", "Chat", QT_TRANSLATE_NOOP("@default", "Link folding"), 0, Advanced);
	ConfigDialog::addSpinBox("Chat", "Link folding", QT_TRANSLATE_NOOP("@default", "Automatically fold links longer than"), "LinkFoldTreshold", 1,500,1,50);

// pierwsze uruchomienie kadu
	config_file.addVariable("Look", "ChatBgColor", QColor("#ffffff"));
	config_file.addVariable("Look", "ChatMyBgColor", QColor("#E0E0E0"));
	config_file.addVariable("Look", "ChatTextBgColor", QColor("#ffffff"));
	config_file.addVariable("Look", "ChatUsrBgColor", QColor("#F0F0F0"));
	config_file.addVariable("Look", "ChatMyFontColor", QColor("#000000"));
	config_file.addVariable("Look", "ChatUsrFontColor", QColor("#000000"));
	config_file.addVariable("Look", "ChatMyNickColor", QColor("#000000"));
	config_file.addVariable("Look", "ChatUsrNickColor", QColor("#000000"));

	//naglowki wiadomosci
	config_file.addVariable("Look", "NoHeaderRepeat", false);
	config_file.addVariable("Look", "HeaderSeparatorHeight", 1);
	config_file.addVariable("Look", "NoHeaderInterval", "10");

	config_file.addVariable("Look", "ChatFont", defaultFont);

	config_file.addVariable("Chat", "LastImagePath", QString(getenv("HOME"))+"/");

	config_file.addVariable("Look", "Style", "kadu");

	ConfigDialog::addVBox("Look", "Look", "varOpts2");//potrzebne userboksowi

//	ConfigDialog::addVGroupBox("Look", "Look", QT_TRANSLATE_NOOP("@default", "Colors"), 0, Advanced);
		ConfigDialog::addVGroupBox("Look", "Colors", QT_TRANSLATE_NOOP("@default", "Chat window"));
			ConfigDialog::addColorButton("Look", "Chat window", QT_TRANSLATE_NOOP("@default", "Chat window background color"), "ChatBgColor", QColor(), 0, "bg_color");
			ConfigDialog::addColorButton("Look", "Chat window", QT_TRANSLATE_NOOP("@default", "Text edit background color"), "ChatTextBgColor", QColor(), 0, "text_bg_color");
			ConfigDialog::addColorButton("Look", "Chat window", QT_TRANSLATE_NOOP("@default", "Your background color"), "ChatMyBgColor", QColor(), 0, "own_bg_color");
			ConfigDialog::addColorButton("Look", "Chat window", QT_TRANSLATE_NOOP("@default", "User background color"), "ChatUsrBgColor", QColor(), 0, "his_bg_color");
			ConfigDialog::addColorButton("Look", "Chat window", QT_TRANSLATE_NOOP("@default", "Your font color"), "ChatMyFontColor", QColor(), 0, "own_font_color");
			ConfigDialog::addColorButton("Look", "Chat window", QT_TRANSLATE_NOOP("@default", "User font color"), "ChatUsrFontColor", QColor(), 0, "his_font_color");
			ConfigDialog::addColorButton("Look", "Chat window", QT_TRANSLATE_NOOP("@default", "Your nick color"), "ChatMyNickColor", QColor(), 0, "own_nick_color");
			ConfigDialog::addColorButton("Look", "Chat window", QT_TRANSLATE_NOOP("@default", "User nick color"), "ChatUsrNickColor", QColor(), 0, "his_nick_color");

//	ConfigDialog::addVGroupBox("Look", "Look", QT_TRANSLATE_NOOP("@default", "Fonts"), 0, Advanced);
		ConfigDialog::addSelectFont("Look", "Fonts", QT_TRANSLATE_NOOP("@default", "Font in chat window"), "ChatFont", defaultFont->toString(), 0, "chat_font_box");

	ConfigDialog::addVGroupBox("Look", "Look", QT_TRANSLATE_NOOP("@default", "Previews"), 0, Advanced);
		ConfigDialog::addVGroupBox("Look", "Previews", QT_TRANSLATE_NOOP("@default", "Chat preview"));
			ConfigDialog::addHBox("Look", "Chat preview", "chat_prvw");
				ConfigDialog::addLabel("Look", "chat_prvw", QT_TRANSLATE_NOOP("@default", "<b>Me</b> 00:00:00"), "chat_me");
				ConfigDialog::addLabel("Look", "chat_prvw", QT_TRANSLATE_NOOP("@default", "<b>Other party</b> 00:00:02"), "chat_other");

	ConfigDialog::addVGroupBox("Look", "Look", QT_TRANSLATE_NOOP("@default","Style"));
		ConfigDialog::addComboBox("Look", "Style", QT_TRANSLATE_NOOP("@default", "Select chat style"), "Style", toStringList("Kadu", "Hapi", "IRC", tr("Custom")), toStringList("kadu", "hapi", "irc", "custom"));
		ConfigDialog::addTextEdit("Look", "Style", QT_TRANSLATE_NOOP("@default", "Full chat style:"), "FullStyle", 0,
			QT_TRANSLATE_NOOP("@default", "Syntax:\n%1 - background color\n%2 - text font color\n%3 - nick color\n%4 - nick\n%5 - timestamp\n%6 - timestamp with server time\n%7 - message"), 0, Expert);

	//naglowki
	ConfigDialog::addVGroupBox("Look", "Look", QT_TRANSLATE_NOOP("@default", "Headers"), 0, Advanced);
		ConfigDialog::addCheckBox("Look", "Headers", QT_TRANSLATE_NOOP("@default", "Remove chat header repetitions"), "NoHeaderRepeat", true, 0, 0, Advanced);
		ConfigDialog::addSpinBox("Look", "Headers", QT_TRANSLATE_NOOP("@default", "Chat header separators height:"), "HeaderSeparatorHeight", 0, config_file.readNumEntry("General", "ParagraphSeparator"), 1, 1, 0, 0, Expert);
		ConfigDialog::addSpinBox("Look", "Headers", QT_TRANSLATE_NOOP("@default", "Interval between header removal:"), "NoHeaderInterval", 1, 1439, 1, 10, 0, 0, Expert);

	ConfigDialog::addVGroupBox("Look", "Look", QT_TRANSLATE_NOOP("@default", "Other"), 0, Expert);
		ConfigDialog::addLineEdit("Look", "Other", QT_TRANSLATE_NOOP("@default", "Chat window title syntax:"), "ChatContents", QString::null, Kadu::SyntaxText, 0, Expert);
		ConfigDialog::addHBox("Look", "Other", "conference", 0, Expert);
			ConfigDialog::addLineEdit("Look", "conference", QT_TRANSLATE_NOOP("@default", "Conference window title prefix:"), "ConferencePrefix", QString::null, QT_TRANSLATE_NOOP("@default", "This text will be before syntax.\nIf you leave blank, default settings will be used."));
			ConfigDialog::addLineEdit("Look", "conference", QT_TRANSLATE_NOOP("@default", "syntax:"), "ConferenceContents", "%a (%s[: %d])", Kadu::SyntaxText);

	config_file.addVariable("Chat", "EmoticonsStyle", EMOTS_ANIMATED);
	emoticons->setEmoticonsTheme(config_file.readEntry("Chat", "EmoticonsTheme"));

	chatslots =new ChatManagerSlots(kadu, "chat_slots");
	ConfigDialog::registerSlotOnCreateTab("Chat", chatslots, SLOT(onCreateTabChat()));
	ConfigDialog::registerSlotOnCreateTab("Look", chatslots, SLOT(onCreateTabLook()));
	ConfigDialog::registerSlotOnApplyTab("Chat", chatslots,SLOT(onApplyTabChat()));
	ConfigDialog::registerSlotOnApplyTab("Look", chatslots,SLOT(onApplyTabLook()));
	ConfigDialog::connectSlot("Chat", "Emoticons:", SIGNAL(activated(int)), chatslots, SLOT(chooseEmoticonsStyle(int)));
	ConfigDialog::connectSlot("Chat", "Automatically prune chat messages", SIGNAL(toggled(bool)), chatslots, SLOT(onPruneChat(bool)));
	ConfigDialog::connectSlot("Chat", "Automatically fold links", SIGNAL(toggled(bool)), chatslots, SLOT(onFoldLink(bool)));
	ConfigDialog::connectSlot("Chat", "Block window close on new message", SIGNAL(toggled(bool)), chatslots, SLOT(onBlockClose(bool)));

	ConfigDialog::connectSlot("Look", "Remove chat header repetitions", SIGNAL(toggled(bool)), chatslots, SLOT(onRemoveHeaders(bool)));

	ConfigDialog::connectSlot("Look", "Your background color", SIGNAL(changed(const char *, const QColor&)),
		chatslots, SLOT(chooseColor(const char *, const QColor&)), "own_bg_color");
	ConfigDialog::connectSlot("Look", "User background color", SIGNAL(changed(const char *, const QColor&)),
		chatslots, SLOT(chooseColor(const char *, const QColor&)), "his_bg_color");
	ConfigDialog::connectSlot("Look", "Your font color", SIGNAL(changed(const char *, const QColor&)),
		chatslots, SLOT(chooseColor(const char *, const QColor&)), "own_font_color");
	ConfigDialog::connectSlot("Look", "User font color", SIGNAL(changed(const char *, const QColor&)),
		chatslots, SLOT(chooseColor(const char *, const QColor&)), "his_font_color");

	ConfigDialog::connectSlot("Look", "Font in chat window", SIGNAL(changed(const char *, const QFont&)), chatslots, SLOT(chooseFont(const char *, const QFont&)), "chat_font_box");
	ConfigDialog::connectSlot("Look", "Select chat style", SIGNAL(activated(const QString&)), chatslots, SLOT(onChatThemeChanged(const QString&)));

	chat_manager=new ChatManager(kadu, "chat_manager");
	connect(gadu, SIGNAL(chatMsgReceived1(Protocol *, UserListElements, const QString&, time_t, bool&)),
		chat_manager, SLOT(chatMsgReceived(Protocol *, UserListElements, const QString&, time_t, bool&)));
	kdebugf2();
}

ChatManager* chat_manager=NULL;
