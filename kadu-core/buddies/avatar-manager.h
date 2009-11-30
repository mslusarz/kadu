/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef AVATAR_MANAGER_H
#define AVATAR_MANAGER_H

#include <QtCore/QObject>
#include <QtGui/QPixmap>

#include "accounts/accounts-aware-object.h"
#include "buddies/avatar.h"
#include "storage/simple-manager.h"

class AvatarService;
class Contact;

class AvatarManager : public QObject, public SimpleManager<Avatar>, AccountsAwareObject
{
	Q_OBJECT
	Q_DISABLE_COPY(AvatarManager)

	static AvatarManager *Instance;

	AvatarManager();
	virtual ~AvatarManager();

	AvatarService * avatarService(Account account);
	AvatarService * avatarService(Contact contact);

	QString avatarFileName(Avatar avatar);

private slots:
	void avatarFetched(Contact contact, const QByteArray &data);

protected:
	virtual void accountRegistered(Account account);
	virtual void accountUnregistered(Account account);

	virtual QString configurationNodeName() { return QLatin1String("Avatars"); }
	virtual QString configurationNodeItemName() { return QLatin1String("Avatar"); }

	virtual void itemAboutToBeAdded(Avatar item);
	virtual void itemAdded(Avatar item);
	virtual void itemAboutToBeRemoved(Avatar item);
	virtual void itemRemoved(Avatar item);

public:
	static AvatarManager * instance();

	void updateAvatar(Contact contact);

signals:
	void avatarAboutToBeAdded(Avatar avatar);
	void avatarAdded(Avatar avatar);
	void avatarAboutToBeRemoved(Avatar avatar);
	void avatarRemoved(Avatar avatar);

	void avatarUpdated(Contact contact);

};

// for MOC
#include "contacts/contact.h"

#endif // AVATAR_MANAGER_H
