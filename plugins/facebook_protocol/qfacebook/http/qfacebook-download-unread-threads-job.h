/*
 * %kadu copyright begin%
 * Copyright 2017 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include "qfacebook/qfacebook-json-reader.h"
#include "qfacebook/session/qfacebook-session-token.h"

#include <QtCore/QObject>
#include <experimental/optional>

class QFacebookHttpApi;
struct QFacebookDownloadUnreadThreadsResult;

class QFacebookDownloadUnreadThreadsJob : public QObject
{
    Q_OBJECT

public:
    explicit QFacebookDownloadUnreadThreadsJob(
        QFacebookHttpApi &httpApi, QFacebookSessionToken facebookSessionToken, int unreadCount,
        QObject *parent = nullptr);
    virtual ~QFacebookDownloadUnreadThreadsJob();

signals:
    void finished(const QFacebookDownloadUnreadThreadsResult &result);

private:
    QFacebookHttpApi &m_httpApi;
    QFacebookSessionToken m_facebookSession;

    void replyFinished(const std::experimental::optional<QFacebookJsonReader> &result);
};
