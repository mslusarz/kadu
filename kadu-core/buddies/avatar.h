/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef AVATAR_H
#define AVATAR_H

#include <QtCore/QDateTime>
#include <QtGui/QPixmap>

#include "buddies/avatar-shared.h"
#include "storage/shared-base.h"
#include "exports.h"

class Contact;
class StoragePoint;

class KADUAPI Avatar : public SharedBase<AvatarShared>
{
	KaduSharedBaseClass(Avatar)

public:
	static Avatar create();
	static Avatar loadFromStorage(StoragePoint *storage);
	static Avatar null;

	Avatar();
	Avatar(AvatarShared *data);
	Avatar(const Avatar &copy);
	virtual ~Avatar();

	QString filePath();
	void setFilePath(const QString &filePath);

	KaduSharedBase_PropertyBoolRead(Empty)
	KaduSharedBase_Property(Contact, avatarContact, AvatarContact)
	KaduSharedBase_Property(QDateTime, lastUpdated, LastUpdated)
	KaduSharedBase_Property(QDateTime, nextUpdate, NextUpdate)
	KaduSharedBase_Property(QPixmap, pixmap, Pixmap)

};

#endif // AVATAR_H
