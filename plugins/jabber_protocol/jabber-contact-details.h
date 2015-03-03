/*
 * %kadu copyright begin%
 * Copyright 2011 Bartosz Brachaczek (b.brachaczek@gmail.com)
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

#ifndef JABBER_CONTACT_DETAILS_H
#define JABBER_CONTACT_DETAILS_H

class QDomDocument;
class QDomElement;

#include "contacts/contact-details.h"
#include "contacts/contact.h"

class JabberContactDetails : public ContactDetails
{
	// PROPERTY_DEC(Subscription, ContactSubscription)

public:
	explicit JabberContactDetails(ContactShared *contactShared);
	virtual ~JabberContactDetails();

	// PROPERTY_DEF(Subscription, subscription, setSubscription, ContactSubscription)

};

#endif // JABBER_CONTACT_DETAILS_H
