/*
 * %kadu copyright begin%
 * Copyright 2010, 2011 Piotr Galiszewski (piotr.galiszewski@kadu.im)
 * Copyright 2009 Wojciech Treter (juzefwt@gmail.com)
 * Copyright 2009, 2010 Tomasz Rostański (rozteck@interia.pl)
 * Copyright 2011, 2013 Rafał Malinowski (rafal.przemyslaw.malinowski@gmail.com)
 * Copyright 2010 Bartosz Brachaczek (b.brachaczek@gmail.com)
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

#ifndef OTR_CREATE_PRIVATE_KEY_JOB_H
#define OTR_CREATE_PRIVATE_KEY_JOB_H

#include <QtCore/QObject>
#include <QtCore/QWeakPointer>

class Account;
class OtrUserStateService;

class OtrCreatePrivateKeyJob : public QObject
{
	Q_OBJECT

	QWeakPointer<OtrUserStateService> UserStateService;

	Account MyAccount;
	QString PrivateStoreFileName;
	QWeakPointer<QThread> CreationThread;
	void *KeyPointer;

private slots:
	void workerFinished(bool ok);

public:
	explicit OtrCreatePrivateKeyJob(QObject *parent = 0);
	virtual ~OtrCreatePrivateKeyJob();

	void setUserStateService(OtrUserStateService *userStateService);

	void setAccount(const Account &account);
	void setPrivateStoreFileName(const QString &privateStoreFileName);
	void createPrivateKey();

signals:
	void finished(const Account &account, bool ok);

};

#endif // OTR_CREATE_PRIVATE_KEY_JOB_H
