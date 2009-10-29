/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "configuration/configuration-manager.h"
#include "contacts/account-data/contact-account-data.h"
#include "core/core.h"
#include "debug.h"

#include "contact-account-data-manager.h"
#include <protocols/protocol.h>
#include <protocols/protocol-factory.h>

ContactAccountDataManager * ContactAccountDataManager::Instance = 0;

ContactAccountDataManager * ContactAccountDataManager::instance()
{
	if (0 == Instance)
	{
		Instance = new ContactAccountDataManager();
		Instance->init();
	}

	return Instance;
}

ContactAccountDataManager::ContactAccountDataManager()
{
	Core::instance()->configuration()->registerStorableObject(this);
}

ContactAccountDataManager::~ContactAccountDataManager()
{
	Core::instance()->configuration()->unregisterStorableObject(this);

	triggerAllAccountsUnregistered();
}

void ContactAccountDataManager::init()
{
	triggerAllAccountsRegistered();
}

StoragePoint * ContactAccountDataManager::createStoragePoint()
{
	return new StoragePoint(xml_config_file, xml_config_file->getNode("ContactAccountDatas"));
}

void ContactAccountDataManager::load(Account account)
{
	if (!isValidStorage())
		return;

	XmlConfigFile *configurationStorage = storage()->storage();
	QDomElement contactAccountDatasNode = storage()->point();

	if (contactAccountDatasNode.isNull())
		return;

	// TODO 0.6.6: by tag does not work, this works only if childNodes are "Chat"
	QDomNodeList contactAccountDatasNodes = contactAccountDatasNode.childNodes();

	int count = contactAccountDatasNodes.count();

	QString uuid = account.uuid().toString();
	for (int i = 0; i < count; i++)
	{
		QDomElement contactAccountDataElement = contactAccountDatasNodes.at(i).toElement();
		if (contactAccountDataElement.isNull())
			continue;

		if (configurationStorage->getTextNode(contactAccountDataElement, "Account") != uuid)
			continue;

		StoragePoint *contactStoragePoint = new StoragePoint(configurationStorage, contactAccountDataElement);
		if (!account.protocolHandler())
			return;

		ContactAccountData *cad = account.protocolHandler()->protocolFactory()->loadContactAccountData(contactStoragePoint);

		if (cad)
		{
			addContactAccountData(cad);
			cad->ensureLoaded();
		}
	}
}

void ContactAccountDataManager::accountRegistered(Account account)
{
	if (LoadedAccounts.contains(account))
		return;

	load(account);
	LoadedAccounts.append(account);
}

void ContactAccountDataManager::accountUnregistered(Account account)
{
// 	store(account);
	LoadedAccounts.removeAll(account);
}

void ContactAccountDataManager::ensureLoaded(Account account)
{
	accountRegistered(account);
}

void ContactAccountDataManager::load()
{
	StorableObject::load();

	if (!isValidStorage())
		return;

	// TODO: implement
}

void ContactAccountDataManager::store()
{
	if (!isValidStorage())
		return;

	StorableObject::ensureLoaded();

	foreach (ContactAccountData *cad, ContactAccountDataList)
		cad->store();
}

void ContactAccountDataManager::addContactAccountData(ContactAccountData *cad)
{
	if (!cad)
		return;

	StorableObject::ensureLoaded();

	if (ContactAccountDataList.contains(cad))
		return;

	emit contactAccountDataAboutToBeAdded(cad);
	ContactAccountDataList.append(cad);
	emit contactAccountDataAdded(cad);
}

void ContactAccountDataManager::removeContactAccountData(ContactAccountData *cad)
{
	kdebugf();

	if (!cad)
		return;

	StorableObject::ensureLoaded();

	if (!ContactAccountDataList.contains(cad))
		return;

	emit contactAccountDataAboutToBeRemoved(cad);
	ContactAccountDataList.removeAll(cad);
	emit contactAccountDataRemoved(cad);
}

ContactAccountData * ContactAccountDataManager::byIndex(unsigned int index)
{
	if (index < 0 || index >= count())
		return 0;

	StorableObject::ensureLoaded();

	return ContactAccountDataList.at(index);
}

ContactAccountData * ContactAccountDataManager::byUuid(const QString &uuid)
{
	if (uuid.isEmpty())
		return 0;

	StorableObject::ensureLoaded();

	foreach (ContactAccountData *cad, ContactAccountDataList)
		if (uuid == cad->uuid().toString())
			return cad;

	return 0;
}
