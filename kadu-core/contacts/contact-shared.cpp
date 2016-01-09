/*
 * %kadu copyright begin%
 * Copyright 2009, 2010, 2011 Piotr Galiszewski (piotr.galiszewski@kadu.im)
 * Copyright 2009 Bartłomiej Zimoń (uzi18@o2.pl)
 * Copyright 2010, 2011, 2012, 2013, 2014 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2009, 2010, 2011, 2012, 2013, 2014 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include "accounts/account-manager.h"
#include "accounts/account.h"
#include "avatars/avatar-manager.h"
#include "avatars/avatar.h"
#include "buddies/buddy-manager.h"
#include "configuration/configuration.h"
#include "configuration/deprecated-configuration-api.h"
#include "contacts/contact-manager.h"
#include "core/application.h"
#include "core/core.h"
#include "misc/change-notifier.h"
#include "protocols/protocol-factory.h"
#include "protocols/protocol.h"
#include "protocols/protocols-manager.h"
#include "roster/roster-entry-state.h"
#include "roster/roster-entry.h"

#include "contact-shared.h"

ContactShared * ContactShared::loadStubFromStorage(const std::shared_ptr<StoragePoint> &storagePoint)
{
	ContactShared *result = loadFromStorage(storagePoint);
	result->loadStub();

	return result;
}

ContactShared * ContactShared::loadFromStorage(const std::shared_ptr<StoragePoint> &storagePoint)
{
	ContactShared *result = new ContactShared();
	result->setStorage(storagePoint);

	return result;
}

ContactShared::ContactShared(const QUuid &uuid) :
		Shared(uuid),
		Priority(-1), MaximumImageSize(0), UnreadMessagesCount(0),
		Blocking(false), IgnoreNextStatusChange(false)
{
	Entry = new RosterEntry(this);
	connect(&Entry->hasLocalChangesNotifier(), SIGNAL(changed()), this, SIGNAL(updatedLocally()));

	ContactAccount = new Account();
	ContactAvatar = new Avatar();
	OwnerBuddy = new Buddy();

	connect(Core::instance()->protocolsManager(), SIGNAL(protocolFactoryRegistered(ProtocolFactory*)),
	        this, SLOT(protocolFactoryRegistered(ProtocolFactory*)));
	connect(Core::instance()->protocolsManager(), SIGNAL(protocolFactoryUnregistered(ProtocolFactory*)),
	        this, SLOT(protocolFactoryUnregistered(ProtocolFactory*)));

	connect(&changeNotifier(), SIGNAL(changed()), this, SLOT(changeNotifierChanged()));
}

ContactShared::~ContactShared()
{
	ref.ref();

	disconnect(Core::instance()->protocolsManager(), 0, this, 0);

	protocolFactoryUnregistered(Core::instance()->protocolsManager()->byName(ContactAccount->protocolName()));

	delete OwnerBuddy;
	delete ContactAvatar;
	delete ContactAccount;
}

StorableObject * ContactShared::storageParent()
{
	return ContactManager::instance();
}

QString ContactShared::storageNodeName()
{
	return QLatin1String("Contact");
}

void ContactShared::load()
{
	if (!isValidStorage())
		return;

	Shared::load();

	Id = loadValue<QString>("Id");
	Priority = loadValue<int>("Priority", -1);

	// TODO: remove after 01.05.2015
	// It's an explicit hack for update path from 0.10.1-0.11.x to 0.12+. 0.10/0.11 didn't
	// have Detached property. But they did have an explicit hack for totally ignoring
	// what Facebook says about groups, thus allowing users to place their Facebook contacts
	// in groups in Kadu. And without below hack all this group information is overriden
	// by useless a Facebook-provided group until we try to upload something to roster
	// for the first time, we fail and only then we set Detached to true, when group
	// information is already lost.
	bool detached = hasValue("Detached")
		? loadValue<bool>("Detached", false)
		: Id.endsWith(QLatin1String("@chat.facebook.com"));
	bool dirty = loadValue<bool>("Dirty", true);
	if (detached)
		Entry->setDetached();
	else if (dirty)
		Entry->setHasLocalChanges();
	else
		Entry->setSynchronized();

	*ContactAccount = Core::instance()->accountManager()->byUuid(loadValue<QString>("Account"));
	doSetOwnerBuddy(Core::instance()->buddyManager()->byUuid(loadValue<QString>("Buddy")));
	doSetContactAvatar(AvatarManager::instance()->byUuid(loadValue<QString>("Avatar")));

	protocolFactoryRegistered(Core::instance()->protocolsManager()->byName(ContactAccount->protocolName()));
	addToBuddy();
}

void ContactShared::aboutToBeRemoved()
{
	// clean up references
	*ContactAccount = Account::null;
	removeFromBuddy();
	doSetOwnerBuddy(Buddy::null);

	AvatarManager::instance()->removeItem(*ContactAvatar);
	doSetContactAvatar(Avatar::null);

	changeNotifier().notify();
}

void ContactShared::store()
{
	if (!isValidStorage())
		return;

	ensureLoaded();

	Shared::store();

	storeValue("Id", Id);
	storeValue("Priority", Priority);

	storeValue("Dirty", RosterEntryState::Synchronized != Entry->state());
	// Detached property needs to be always stored, see the load() method.
	storeValue("Detached", RosterEntryState::Detached == Entry->state());

	storeValue("Account", ContactAccount->uuid().toString());
	storeValue("Buddy", !isAnonymous()
			? OwnerBuddy->uuid().toString()
			: QString());

	if (*ContactAvatar)
		storeValue("Avatar", ContactAvatar->uuid().toString());

	removeValue("Contact");
}

bool ContactShared::shouldStore()
{
	ensureLoaded();

	if (!UuidStorableObject::shouldStore())
		return false;

	if (Id.isEmpty() || ContactAccount->uuid().isNull())
		return false;

	// we dont need data for non-roster contacts only from 4 version of sql schema
	if (Application::instance()->configuration()->deprecatedApi()->readNumEntry("History", "Schema", 0) < 4)
		return true;

	return !isAnonymous() || rosterEntry()->requiresSynchronization() || customProperties()->shouldStore();
}

void ContactShared::addToBuddy()
{
	// dont add to buddy if details are not available
	if (*OwnerBuddy)
		OwnerBuddy->addContact(this);
}

void ContactShared::removeFromBuddy()
{
	if (*OwnerBuddy)
		OwnerBuddy->removeContact(this);
}

void ContactShared::setOwnerBuddy(const Buddy &buddy)
{
	ensureLoaded();

	if (*OwnerBuddy == buddy)
		return;

	/* NOTE: This guard is needed to avoid deleting this object when removing
	 * Contact from Buddy which may hold last reference to it and thus wants to
	 * delete it. But we don't want this to happen.
	 */
	Contact guard(this);

	removeFromBuddy();
	doSetOwnerBuddy(buddy);
	addToBuddy();

	Entry->setHasLocalChanges();
	changeNotifier().notify();

	emit buddyUpdated();
}

void ContactShared::setContactAccount(const Account &account)
{
	ensureLoaded();

	if (*ContactAccount == account)
		return;

	if (*ContactAccount && ContactAccount->protocolHandler() && ContactAccount->protocolHandler()->protocolFactory())
		protocolFactoryUnregistered(ContactAccount->protocolHandler()->protocolFactory());

	*ContactAccount = account;

	if (*ContactAccount && ContactAccount->protocolHandler() && ContactAccount->protocolHandler()->protocolFactory())
		protocolFactoryRegistered(ContactAccount->protocolHandler()->protocolFactory());

	changeNotifier().notify();
}

void ContactShared::protocolFactoryRegistered(ProtocolFactory *protocolFactory)
{
	ensureLoaded();

	if (!protocolFactory || !*ContactAccount || ContactAccount->protocolName() != protocolFactory->name())
		return;

	changeNotifier().notify();
}

void ContactShared::protocolFactoryUnregistered(ProtocolFactory *protocolFactory)
{
	ensureLoaded();

	if (!protocolFactory || ContactAccount->protocolName() != protocolFactory->name())
		return;

	/* NOTE: This guard is needed to avoid deleting this object when detaching
	 * Contact from Buddy which may hold last reference to it and thus wants to
	 * delete it. But we don't want this to happen.
	 */
	Contact guard(this);
	changeNotifier().notify();
}

void ContactShared::setId(const QString &id)
{
	ensureLoaded();

	if (Id == id)
		return;

	QString oldId = Id;
	Id = id;

	changeNotifier().notify();
}

RosterEntry * ContactShared::rosterEntry()
{
	ensureLoaded();

	return Entry;
}

/*
 * @todo: move this comment somewhere
 *
 * Sets state if this contact to \p dirty. All contacts are dirty by default.
 *
 * Dirty contacts with anonymous owner buddies are considered dirty removed and will
 * never be added to roster as long as this state lasts and will in effect be removed
 * from remote roster. Dirty contacts with not anonymous owner buddies are considered
 * dirty added and will always be added to roster, even if remote roster marked
 * them as removed.
 *
 * When adding contacts with anononymous owner buddies to the manager, always make sure
 * to mark them not dirty, otherwise they will be considered dirty removed and will
 * not be added to roster if remote roster says so, which is probably not what one expects.
 */

void ContactShared::avatarUpdated()
{
	changeNotifier().notify();
}

void ContactShared::changeNotifierChanged()
{
	emit updated();
}

void ContactShared::doSetOwnerBuddy(const Buddy &buddy)
{
	if (*OwnerBuddy)
		disconnect(*OwnerBuddy, 0, this, 0);

	*OwnerBuddy = buddy;

	if (*OwnerBuddy)
		connect(*OwnerBuddy, SIGNAL(updated()), this, SIGNAL(buddyUpdated()));
}

void ContactShared::doSetContactAvatar(const Avatar &contactAvatar)
{
	if (*ContactAvatar)
		disconnect(*ContactAvatar, 0, this, 0);

	*ContactAvatar = contactAvatar;

	if (*ContactAvatar)
		connect(*ContactAvatar, SIGNAL(updated()), this, SLOT(avatarUpdated()));
}

void ContactShared::setContactAvatar(const Avatar &contactAvatar)
{
	ensureLoaded();

	if (*ContactAvatar == contactAvatar)
		return;

	doSetContactAvatar(contactAvatar);

	changeNotifier().notify();
}

void ContactShared::setPriority(int priority)
{
	ensureLoaded();
	if (Priority != priority)
	{
		Priority = priority;
		changeNotifier().notify();
		emit priorityUpdated();
	}
}

bool ContactShared::isAnonymous()
{
	ensureLoaded();

	if (!OwnerBuddy)
		return true;

	if (!(*OwnerBuddy))
		return true;

	return OwnerBuddy->isAnonymous();
}

QString ContactShared::display(bool useBuddyData)
{
	ensureLoaded();

	if (!useBuddyData || !OwnerBuddy || !(*OwnerBuddy) || OwnerBuddy->display().isEmpty())
		return Id;

	return OwnerBuddy->display();
}

Avatar ContactShared::avatar(bool useBuddyData)
{
	ensureLoaded();

	if (!useBuddyData || !OwnerBuddy || !(*OwnerBuddy) || OwnerBuddy->buddyAvatar().isEmpty())
		return ContactAvatar ? *ContactAvatar : Avatar::null;

	return OwnerBuddy->buddyAvatar();
}

KaduShared_PropertyPtrReadDef(ContactShared, Account, contactAccount, ContactAccount)
KaduShared_PropertyPtrReadDef(ContactShared, Avatar, contactAvatar, ContactAvatar)
KaduShared_PropertyPtrReadDef(ContactShared, Buddy, ownerBuddy, OwnerBuddy)

#include "moc_contact-shared.cpp"
