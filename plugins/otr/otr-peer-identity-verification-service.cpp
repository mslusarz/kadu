/*
 * %kadu copyright begin%
 * Copyright 2013 Rafał Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include "contacts/contact.h"

#include "otr-peer-identity-verification-job.h"

#include "otr-peer-identity-verification-service.h"

OtrPeerIdentityVerificationService::OtrPeerIdentityVerificationService(QObject *parent) :
		QObject(parent)
{
}

OtrPeerIdentityVerificationService::~OtrPeerIdentityVerificationService()
{
}

OtrPeerIdentityVerificationJob * OtrPeerIdentityVerificationService::verificationJobForContact(const Contact &contact)
{
	if (VerificationJobs.contains(contact))
		return VerificationJobs.value(contact);
	
	
	OtrPeerIdentityVerificationJob *result = new OtrPeerIdentityVerificationJob(contact);
	connect(result, SIGNAL(destroyed(Contact)), this, SLOT(jobDestroyed(Contact)));
	VerificationJobs.insert(contact, result);

	return result;
}

void OtrPeerIdentityVerificationService::jobDestroyed(const Contact &contact)
{
	VerificationJobs.remove(contact);
}
