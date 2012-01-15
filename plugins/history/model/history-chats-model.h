/*
 * %kadu copyright begin%
 * Copyright 2009, 2010, 2011 Piotr Galiszewski (piotr.galiszewski@kadu.im)
 * Copyright 2009 Wojciech Treter (juzefwt@gmail.com)
 * Copyright 2009, 2010, 2011 Rafał Malinowski (rafal.przemyslaw.malinowski@gmail.com)
 * Copyright 2010, 2011 Bartosz Brachaczek (b.brachaczek@gmail.com)
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

#ifndef HISTORY_CHATS_MODEL_H
#define HISTORY_CHATS_MODEL_H

#include <QtCore/QAbstractItemModel>
#include <QtCore/QMap>
#include <QtCore/QVector>

class Buddy;

class HistoryChatsModel : public QAbstractItemModel
{
	Q_OBJECT

	QList<QString> SmsRecipients;

	void clearSmsRecipients();

public:
	HistoryChatsModel(QObject *parent = 0);
	virtual ~HistoryChatsModel();

	virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;

	virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
	virtual QModelIndex parent(const QModelIndex &child) const;

	virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

	void setSmsRecipients(const QList<QString> &smsRecipients);

	QModelIndex smsRecipientIndex(const QString &recipient) const;

};

#endif // HISTORY_CHATS_MODEL_H
