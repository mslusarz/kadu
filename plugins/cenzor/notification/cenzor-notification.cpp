/*
 * %kadu copyright begin%
 * Copyright 2011, 2013 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2011, 2013, 2014, 2015 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include "core/injected-factory.h"
#include "icons/icons-manager.h"
#include "notification/notification-manager.h"
#include "notification/notification-event.h"

#include "cenzor-notification.h"

void CenzorNotification::notifyCenzored(InjectedFactory *injectedFactory, NotificationManager *notificationManager, const Chat &chat)
{
	CenzorNotification *notification = injectedFactory->makeInjected<CenzorNotification>(chat);
	notification->setTitle(tr("Cenzor"));
	notification->setText(tr("Message was cenzored"));
	notification->setDetails(tr("Your interlocutor used obscene word and became admonished"));
	notificationManager->notify(notification);
}

CenzorNotification::CenzorNotification(const Chat &chat) :
		Notification(Account::null, chat, "cenzorNotification", KaduIcon())
{
	addChatCallbacks();
}

CenzorNotification::~CenzorNotification()
{
}

#include "moc_cenzor-notification.cpp"
