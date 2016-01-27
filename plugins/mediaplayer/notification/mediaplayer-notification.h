/*
 * %kadu copyright begin%
 * Copyright 2012 Wojciech Treter (juzefwt@gmail.com)
 * Copyright 2012 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2013, 2014 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#ifndef MEDIAPLAYER_NOTIFICATION_H
#define MEDIAPLAYER_NOTIFICATION_H

#include "notification/notification/notification.h"

#include "mediaplayer-exports.h"

class InjectedFactory;
class NotificationEventRepository;
class NotificationManager;

class MEDIAPLAYERAPI MediaPlayerNotification : public Notification
{
	Q_OBJECT

public:
	static void registerNotifications(NotificationEventRepository *notificationEventRepository);
	static void unregisterNotifications(NotificationEventRepository *notificationEventRepository);

	static void notifyTitleHint(InjectedFactory *injectedFactory, NotificationManager *notificationManager, const QString &title);

	MediaPlayerNotification();
	virtual ~MediaPlayerNotification();

};

#endif // MEDIAPLAYER_NOTIFICATION_H
