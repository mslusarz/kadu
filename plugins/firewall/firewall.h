/*
 * %kadu copyright begin%
 * Copyright 2008, 2010, 2011 Piotr Galiszewski (piotr.galiszewski@kadu.im)
 * Copyright 2009 Wojciech Treter (juzefwt@gmail.com)
 * Copyright 2008 Tomasz Rostański (rozteck@interia.pl)
 * Copyright 2008, 2010, 2011 Rafał Malinowski (rafal.przemyslaw.malinowski@gmail.com)
 * Copyright 2010 Bartosz Brachaczek (b.brachaczek@gmail.com)
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

#ifndef KADU_FIREWALL_H
#define KADU_FIREWALL_H

#include <QtCore/QDateTime>
#include <QtCore/QObject>
#include <QtCore/QRegExp>

#include "chat/chat.h"
#include "contacts/contact-set.h"

#include "accounts/accounts-aware-object.h"
#include "configuration/configuration-aware-object.h"
#include "message/message-filter.h"

class Account;
class ChatWidget;
class Contact;
class FormattedStringFactory;
class IncomingMessageFirewallFilter;
class Message;
class OutgoingMessageFirewallFilter;

class Firewall : public MessageFilter, ConfigurationAwareObject, AccountsAwareObject
{
	Q_OBJECT

	static Firewall * Instance;

	explicit Firewall();
	virtual ~Firewall();

	QWeakPointer<FormattedStringFactory> CurrentFormattedStringFactory;

	BuddySet SecuredTemporaryAllowed;
	ContactSet Passed;
	Contact LastContact;

	unsigned int FloodMessages;
	QTime LastMsg;
	QTime LastNotify;
	QRegExp pattern;

	bool CheckFloodingEmoticons;
	bool EmoticonsAllowKnown;
	bool WriteLog;
	bool CheckDos;
	bool CheckChats;
	bool IgnoreConferences;
	bool WriteInHistory;
	bool DropAnonymousWhenInvisible;
	bool IgnoreInvisible;
	bool Confirmation;
	bool Search;
	bool SafeSending;
	int DosInterval;
	int MaxEmoticons;
	QString ConfirmationText;
	QString ConfirmationQuestion;
	QString LogFilePath;

	void import_0_6_5_configuration();

	bool checkChat(const Chat &chat, const Contact &sender, const QString &message, bool &stop);
	bool checkConference(const Chat &chat);
	bool checkEmoticons(const QString &message);
	bool checkFlood();

	void writeLog(const Contact &contact, const QString &message);

	void createDefaultConfiguration();

private slots:
	void accountConnected();

	void chatDestroyed(ChatWidget *);

protected:
	virtual void accountRegistered(Account account);
	virtual void accountUnregistered(Account account);
	virtual void configurationUpdated();

public:
	static void createInstance();
	static void destroyInstance();

	static Firewall * instance() { return Instance; }

	void setFormattedStringFactory(FormattedStringFactory *formattedStringFactory);

	virtual bool acceptMessage(const Message& message);

	bool acceptIncomingMessage(const Message &message);
	bool acceptOutgoingMessage(const Message &message);

};

#endif // KADU_FIREWALL_H
