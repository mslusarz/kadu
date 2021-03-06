/*
 * %kadu copyright begin%
 * Copyright 2010, 2011 Piotr Galiszewski (piotr.galiszewski@kadu.im)
 * Copyright 2011 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2010, 2011, 2013, 2014 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include <QtCore/QObject>
#include <QtCore/QPointer>
#include <injeqt/injeqt.h>

class Avatars;
class BuddyPreferredManager;
class Buddy;
class IconsManager;

class QVariant;

class BuddyDataExtractor : public QObject
{
    Q_OBJECT

public:
    Q_INVOKABLE explicit BuddyDataExtractor(QObject *parent = nullptr);
    virtual ~BuddyDataExtractor();

    QVariant decoration(const Buddy &buddy);
    QVariant data(const Buddy &buddy, int role);

private:
    QPointer<Avatars> m_avatars;
    QPointer<BuddyPreferredManager> m_buddyPreferredManager;
    QPointer<IconsManager> m_iconsManager;

private slots:
    INJEQT_SET void setAvatars(Avatars *avatars);
    INJEQT_SET void setBuddyPreferredManager(BuddyPreferredManager *buddyPreferredManager);
    INJEQT_SET void setIconsManager(IconsManager *iconsManager);
};
