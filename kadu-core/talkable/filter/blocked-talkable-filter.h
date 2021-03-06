/*
 * %kadu copyright begin%
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

#ifndef BLOCKED_TALKABLE_FILTER_H
#define BLOCKED_TALKABLE_FILTER_H

#include <QtCore/QMetaType>

#include "talkable/filter/talkable-filter.h"

/**
 * @addtogroup Talkable
 * @{
 */

/**
 * @class BlockedTalkableFilter
 * @short Filter that removes buddies and contacts that are blocked.
 *
 * This filter removes buddies and contacts that are blocked. Non-blocked buddies and contacts and all chats
 * are passed to nexts filters.
 */
class BlockedTalkableFilter : public TalkableFilter
{
    Q_OBJECT

    bool Enabled;

public:
    /**
     * @short Create new instance of BlockedTalkableFilter with given parent.
     * @param parent QObject parent of new object
     */
    explicit BlockedTalkableFilter(QObject *parent = nullptr);
    virtual ~BlockedTalkableFilter();

    virtual FilterResult filterBuddy(const Buddy &buddy);
    virtual FilterResult filterContact(const Contact &contact);

    void setEnabled(bool enabled);
};

/**
 * @addtogroup Talkable
 * @}
 */

Q_DECLARE_METATYPE(BlockedTalkableFilter *)

#endif   // BLOCKED_TALKABLE_FILTER_H
