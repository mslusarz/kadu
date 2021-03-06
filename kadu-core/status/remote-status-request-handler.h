/*
 * %kadu copyright begin%
 * Copyright 2016 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include "injeqt-type-roles.h"

#include <QtCore/QObject>
#include <QtCore/QPointer>
#include <injeqt/injeqt.h>

class AccountManager;
class Account;
class StatusContainerManager;
class StatusSetter;
class Status;

class RemoteStatusRequestHandler : public QObject
{
    Q_OBJECT
    INJEQT_TYPE_ROLE(SERVICE)

public:
    Q_INVOKABLE explicit RemoteStatusRequestHandler(QObject *parent = nullptr);
    virtual ~RemoteStatusRequestHandler();

private:
    QPointer<AccountManager> m_accountManager;
    QPointer<StatusContainerManager> m_statusContainerManager;
    QPointer<StatusSetter> m_statusSetter;

    void accountAdded(Account account);
    void accountRemoved(Account account);
    void remoteStatusChangeRequest(Account account, Status requestedStatus);

private slots:
    INJEQT_SET void setAccountManager(AccountManager *accountManager);
    INJEQT_SET void setStatusContainerManager(StatusContainerManager *statusContainerManager);
    INJEQT_SET void setStatusSetter(StatusSetter *statusSetter);
    INJEQT_INIT void init();
};
