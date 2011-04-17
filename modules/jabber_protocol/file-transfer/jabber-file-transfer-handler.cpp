/*
 * %kadu copyright begin%
 * Copyright 2009, 2009 Wojciech Treter (juzefwt@gmail.com)
 * Copyright 2009, 2010 Rafał Malinowski (rafal.przemyslaw.malinowski@gmail.com)
 * Copyright 2009 Michał Podsiadlik (michal@kadu.net)
 * Copyright 2010 Piotr Galiszewski (piotrgaliszewski@gmail.com)
 * Copyright 2009 Bartłomiej Zimoń (uzi18@o2.pl)
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

#include <QtCore/QFileInfo>

#include <xmpp/xmpp-im/xmpp_bytestream.h>
#include <filetransfer.h>

#include "resource/jabber-resource-pool.h"
#include "jabber-protocol.h"

#include "jabber-file-transfer-handler.h"

JabberFileTransferHandler::JabberFileTransferHandler(::FileTransfer transfer) :
		FileTransferHandler(transfer), JabberTransfer(0), InProgress(false), BytesTransferred(0)
{
}

JabberFileTransferHandler::~JabberFileTransferHandler()
{
}

void JabberFileTransferHandler::connectJabberTransfer()
{
	if (!JabberTransfer)
		return;

	connect(JabberTransfer, SIGNAL(accepted()), this, SLOT(fileTransferAccepted()));
	connect(JabberTransfer, SIGNAL(connected()), this, SLOT(fileTransferConnected()));
	connect(JabberTransfer, SIGNAL(readyRead(const QByteArray &)), this, SLOT(fileTransferReadyRead(const QByteArray &)));
	connect(JabberTransfer, SIGNAL(bytesWritten(int)), this, SLOT(fileTransferBytesWritten(int)));
	connect(JabberTransfer, SIGNAL(error(int)), this, SLOT(fileTransferError(int)));
}

void JabberFileTransferHandler::disconnectJabberTransfer()
{
	if (!JabberTransfer)
		return;

	disconnect(JabberTransfer, SIGNAL(accepted()), this, SLOT(fileTransferAccepted()));
	disconnect(JabberTransfer, SIGNAL(connected()), this, SLOT(fileTransferConnected()));
	disconnect(JabberTransfer, SIGNAL(readyRead(const QByteArray &)), this, SLOT(fileTransferReadyRead(const QByteArray &)));
	disconnect(JabberTransfer, SIGNAL(bytesWritten(int)), this, SLOT(fileTransferBytesWritten(int)));
	disconnect(JabberTransfer, SIGNAL(error(int)), this, SLOT(fileTransferError(int)));
}

void JabberFileTransferHandler::setJTransfer(XMPP::FileTransfer *jTransfer)
{
	disconnectJabberTransfer();
	JabberTransfer = jTransfer;
	connectJabberTransfer();
}

void JabberFileTransferHandler::cleanup(FileTransferStatus status)
{
	transfer().setTransferStatus(status);
	delete JabberTransfer;
	JabberTransfer = 0;

	if (LocalFile.isOpen())
		LocalFile.close();
}

void JabberFileTransferHandler::updateFileInfo()
{
	if (JabberTransfer)
		transfer().setTransferredSize(BytesTransferred);
	else
		transfer().setTransferredSize(0);

	emit statusChanged();
}

void JabberFileTransferHandler::send()
{
	if (TypeSend != transfer().transferType()) // maybe assert here?
		return;

	if (InProgress) // already sending/receiving
		return;

	transfer().setRemoteFileName(transfer().localFileName());

	QFileInfo fileInfo(transfer().localFileName());
	transfer().setFileSize(fileInfo.size());

	Account account = transfer().peer().contactAccount();
	if (account.isNull() || transfer().localFileName().isEmpty())
	{
		transfer().setTransferStatus(StatusNotConnected);
		return; // TODO: notify
	}

	JabberProtocol *jabberProtocol = dynamic_cast<JabberProtocol *>(account.protocolHandler());
	if (!jabberProtocol)
	{
		transfer().setTransferStatus(StatusNotConnected);
		return;
	}

	if (!jabberProtocol->jabberContactDetails(transfer().peer()))
	{
		transfer().setTransferStatus(StatusNotConnected);
		return;
	}

	XMPP::Jid proxy;
	JabberAccountDetails *jabberAccountDetails = dynamic_cast<JabberAccountDetails *>(account.details());
	if (0 != jabberAccountDetails)
		proxy = jabberAccountDetails->dataTransferProxy();

	QString jid = transfer().peer().id();
	// sendFile needs jid with resource so take best from ResourcePool
	PeerJid = XMPP::Jid(jid).withResource(jabberProtocol->resourcePool()->bestResource(jid).name());

	if (!JabberTransfer)
	{
		JabberTransfer = jabberProtocol->xmppClient()->fileTransferManager()->createTransfer();
		connectJabberTransfer();
	}

// 	if (proxy.isValid())
// 		JabberTransfer->setProxy(proxy);

	transfer().setTransferStatus(StatusWaitingForConnection);
	InProgress = true;

	JabberTransfer->sendFile(PeerJid, transfer().localFileName(), transfer().fileSize(), QString());
}

void JabberFileTransferHandler::stop()
{
	if (JabberTransfer)
		JabberTransfer->close();

	cleanup(StatusNotConnected);
}

void JabberFileTransferHandler::pause()
{
	stop();
}

void JabberFileTransferHandler::restore()
{
	if (TypeSend == transfer().transferType())
		send();
}

/**
 * @todo do not pass opened file to this method
 */
bool JabberFileTransferHandler::accept(QFile &file)
{
	// this suxx, I know
	file.close();
	LocalFile.setFileName(file.fileName());

	if (JabberTransfer->rangeSupported())
	{
		if (!LocalFile.open(QIODevice::Append | QIODevice::WriteOnly))
			return false;
	}
	else
	{
		// we have to close file and reopen it
		if (!LocalFile.open(QIODevice::Truncate | QIODevice::WriteOnly))
			return false;
	}

	BytesTransferred = file.size();

	transfer().accept(file);
	transfer().setTransferStatus(StatusTransfer);
	transfer().setTransferredSize(BytesTransferred);

	if (TypeReceive == transfer().transferType())
		transfer().setFileSize(JabberTransfer->fileSize());

	JabberTransfer->accept(BytesTransferred);

	return true;
}

void JabberFileTransferHandler::reject()
{
	if (JabberTransfer)
		JabberTransfer->close();

	deleteLater();
}

void JabberFileTransferHandler::fileTransferAccepted()
{
	transfer().setTransferStatus(StatusTransfer);
}

void JabberFileTransferHandler::fileTransferConnected()
{
	if (TypeSend == transfer().transferType())
	{
		if (!LocalFile.isOpen())
		{
			LocalFile.setFileName(transfer().localFileName());
			if (!LocalFile.open(QIODevice::ReadOnly))
			{
				cleanup(StatusNotConnected);
				return;
			}
		}

		if (!JabberTransfer->bsConnection())
		{
			cleanup(StatusNotConnected);
			return;
		}

		int dataSize = JabberTransfer->dataSizeNeeded();
		QByteArray data(dataSize, (char)0);

		int sizeRead = LocalFile.read(data.data(), data.size());
		if (sizeRead < 0)
		{
			cleanup(StatusNotConnected);
			return;
		}

		if (sizeRead < data.size())
			data.resize(sizeRead);
			
		JabberTransfer->writeFileData(data);
	}
	else
	{
		if (!LocalFile.isOpen())
		{
			LocalFile.setFileName(transfer().localFileName());
			if (!LocalFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
			{
				cleanup(StatusNotConnected);
				return;
			}
		}
	}

	transfer().setTransferStatus(StatusTransfer);
}

void JabberFileTransferHandler::fileTransferReadyRead(const QByteArray &a)
{
	LocalFile.write(a);

	BytesTransferred += a.size();
	updateFileInfo();

	if (BytesTransferred == JabberTransfer->fileSize())
		cleanup(StatusFinished);
}

void JabberFileTransferHandler::fileTransferBytesWritten(int written)
{
	BytesTransferred += written;
	updateFileInfo();

	if (BytesTransferred == (qlonglong)(transfer().fileSize()))
	{
		cleanup(StatusFinished);
		return;
	}

	if (!JabberTransfer->bsConnection())
	{
		cleanup(StatusNotConnected);
		return;
	}

	int dataSize = JabberTransfer->dataSizeNeeded();

	QByteArray data(dataSize, (char)0);

	int sizeRead = LocalFile.read(data.data(), data.size());
	if (sizeRead < 0)
	{
		cleanup(StatusNotConnected);
		return;
	}

	if (sizeRead < data.size())
		data.resize(sizeRead);

	JabberTransfer->writeFileData(data);
}

FileTransferStatus JabberFileTransferHandler::errorToStatus(int error)
{
	switch (error)
	{
		case XMPP::FileTransfer::ErrReject:
			return StatusRejected;
			break;
		case XMPP::FileTransfer::ErrNeg:
		case XMPP::FileTransfer::ErrConnect:
		case XMPP::FileTransfer::ErrStream:
		default:
			return StatusNotConnected;
			break;
	}
}

void JabberFileTransferHandler::fileTransferError(int error)
{
	cleanup(errorToStatus(error));
}
