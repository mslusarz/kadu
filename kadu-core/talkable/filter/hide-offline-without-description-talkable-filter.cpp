/*
 * %kadu copyright begin%
 * Copyright 2011, 2013 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2011, 2013 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include "hide-offline-without-description-talkable-filter.h"
#include "hide-offline-without-description-talkable-filter.moc"

#include "buddies/buddy-preferred-manager.h"
#include "buddies/buddy.h"
#include "contacts/contact.h"

HideOfflineWithoutDescriptionTalkableFilter::HideOfflineWithoutDescriptionTalkableFilter(QObject *parent)
        : TalkableFilter(parent), m_enabled(false)
{
}

HideOfflineWithoutDescriptionTalkableFilter::~HideOfflineWithoutDescriptionTalkableFilter()
{
}

void HideOfflineWithoutDescriptionTalkableFilter::setBuddyPreferredManager(BuddyPreferredManager *buddyPreferredManager)
{
    m_buddyPreferredManager = buddyPreferredManager;
}

TalkableFilter::FilterResult HideOfflineWithoutDescriptionTalkableFilter::filterBuddy(const Buddy &buddy)
{
    if (!m_enabled)
        return Undecided;

    auto const &contact = m_buddyPreferredManager->preferredContact(buddy, false);
    if (!contact)
        return Rejected;

    return filterContact(contact);
}

TalkableFilter::FilterResult HideOfflineWithoutDescriptionTalkableFilter::filterContact(const Contact &contact)
{
    if (!m_enabled)
        return Undecided;

    auto const &status = contact.currentStatus();
    if (status.isDisconnected() && status.description().isEmpty())
        return Rejected;
    else
        return Undecided;
}

void HideOfflineWithoutDescriptionTalkableFilter::setEnabled(bool enabled)
{
    if (m_enabled == enabled)
        return;

    m_enabled = enabled;
    emit filterChanged();
}
