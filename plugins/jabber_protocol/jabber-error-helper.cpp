/*
 * %kadu copyright begin%
 * Copyright 2014 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2012, 2013, 2014 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
 * %kadu copyright end%
 *
 * Copyright (C) 2006 Remko Troncon
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

#include <QtWidgets/QApplication>

#include "jabber-error-helper.h"
/*
void JabberErrorHelper::getErrorInfo(int err, AdvancedConnector *conn, Stream *stream, QCATLSHandler *tlsHandler, QString *_str, bool *_reconn)
{
	QString str;
	bool reconn = false;

	if (err == -1)
	{
		str = QCoreApplication::translate("@default", "Disconnected");
		reconn = true;
	}
	else if (err == ClientStream::ErrParse)
	{
		str = QCoreApplication::translate("@default", "XML Parsing Error");
		reconn = true;
	}
	else if (err == ClientStream::ErrProtocol)
	{
		str = QCoreApplication::translate("@default", "XMPP Protocol Error");
		reconn = true;
	}
	else if (err == ClientStream::ErrStream)
	{
		int x;
		QString s, detail;
		reconn = true;
		if (stream)  // Stream can apparently be gone if you disconnect in time
		{
			x = stream->errorCondition();
			detail = stream->errorText();
		}
		else
		{
			x = Stream::GenericStreamError;
			reconn = false;
		}

		if (x == Stream::GenericStreamError)
			s = QCoreApplication::translate("@default", "Generic stream error");
		else if (x == ClientStream::Conflict)
		{
			s = QCoreApplication::translate("@default", "Conflict(remote login replacing this one)");
			reconn = false;
		}
		else if (x == ClientStream::ConnectionTimeout)
			s = QCoreApplication::translate("@default", "Timed out from inactivity");
		else if (x == ClientStream::InternalServerError)
			s = QCoreApplication::translate("@default", "Internal server error");
		else if (x == ClientStream::InvalidXml)
			s = QCoreApplication::translate("@default", "Invalid XML");
		else if (x == ClientStream::PolicyViolation)
		{
			s = QCoreApplication::translate("@default", "Policy violation");
			reconn = false;
		}
		else if (x == ClientStream::ResourceConstraint)
		{
			s = QCoreApplication::translate("@default", "Server out of resources");
			reconn = false;
		}
		else if (x == ClientStream::SystemShutdown)
		{
			s = QCoreApplication::translate("@default", "Server is shutting down");
		}
		str = QCoreApplication::translate("@default", "XMPP Stream Error: %1").arg(s) + '\n' + detail;
	}
	else if (err == ClientStream::ErrConnection)
	{
		int x = conn->errorCode();
		QString s;
		reconn = true;
		if (x == AdvancedConnector::ErrConnectionRefused)
			s = QCoreApplication::translate("@default", "Unable to connect to server");
		else if (x == AdvancedConnector::ErrHostNotFound)
			s = QCoreApplication::translate("@default", "Host not found");
		else if (x == AdvancedConnector::ErrProxyConnect)
			s = QCoreApplication::translate("@default", "Error connecting to proxy");
		else if (x == AdvancedConnector::ErrProxyNeg)
			s = QCoreApplication::translate("@default", "Error during proxy negotiation");
		else if (x == AdvancedConnector::ErrProxyAuth)
		{
			s = QCoreApplication::translate("@default", "Proxy authentication failed");
			reconn = false;
		}
		else if (x == AdvancedConnector::ErrStream)
			s = QCoreApplication::translate("@default", "Socket/stream error");
		str = QCoreApplication::translate("@default", "Connection Error: %1").arg(s);
	}
	else if (err == ClientStream::ErrNeg)
	{
		QString s, detail;
		int x = stream->errorCondition();
		detail = stream->errorText();
		if (x  == ClientStream::HostGone)
			s = QCoreApplication::translate("@default", "Host no longer hosted");
		else if (x == ClientStream::HostUnknown)
			s = QCoreApplication::translate("@default", "Host unknown");
		else if (x == ClientStream::RemoteConnectionFailed)
		{
			s = QCoreApplication::translate("@default", "A required remote connection failed");
			reconn = true;
		}
		else if (x == ClientStream::SeeOtherHost)
			s = QCoreApplication::translate("@default", "See other host: %1").arg(stream->errorText());
		else if (x == ClientStream::UnsupportedVersion)
			s = QCoreApplication::translate("@default", "Server does not support proper XMPP version");
		str = QCoreApplication::translate("@default", "Stream Negotiation Error: %1").arg(s) + '\n' + detail;
	}
	else if (err == ClientStream::ErrTLS)
	{
		int x = stream->errorCondition();
		QString s;
		if (x == ClientStream::TLSStart)
			s = QCoreApplication::translate("@default", "Server rejected STARTTLS");
		else if (x == ClientStream::TLSFail)
		{
			int t = tlsHandler->tlsError();
			if (t == QCA::TLS::ErrorHandshake)
				s = QCoreApplication::translate("@default", "TLS handshake error");
			else
				s = QCoreApplication::translate("@default", "Broken security layer (TLS)");
		}
		str = s;
	}
	else if (err == ClientStream::ErrAuth)
	{
		int x = stream->errorCondition();
		QString s;
		if (x == ClientStream::GenericAuthError)
			s = QCoreApplication::translate("@default", "Unable to login");
		else if (x == ClientStream::NoMech)
		{
			s = QCoreApplication::translate("@default", "No appropriate mechanism available for given security settings(e.g. SASL library too weak, or plaintext authentication not enabled)");
			s += '\n' + stream->errorText();
		}
		else if (x == ClientStream::BadProto)
			s = QCoreApplication::translate("@default", "Bad server response");
		else if (x == ClientStream::BadServ)
			s = QCoreApplication::translate("@default", "Server failed mutual authentication");
		else if (x == ClientStream::EncryptionRequired)
			s = QCoreApplication::translate("@default", "Encryption required for chosen SASL mechanism");
		else if (x == ClientStream::InvalidAuthzid)
			s = QCoreApplication::translate("@default", "Invalid account information");
		else if (x == ClientStream::InvalidMech)
			s = QCoreApplication::translate("@default", "Invalid SASL mechanism");
		else if (x == ClientStream::InvalidRealm)
			s = QCoreApplication::translate("@default", "Invalid realm");
		else if (x == ClientStream::MechTooWeak)
			s = QCoreApplication::translate("@default", "SASL mechanism too weak for this account");
		else if (x == ClientStream::NotAuthorized)
			s = QCoreApplication::translate("@default", "Not authorized");
		else if (x == ClientStream::TemporaryAuthFailure)
			s = QCoreApplication::translate("@default", "Temporary auth failure");

		str = QCoreApplication::translate("@default", "Authentication error: %1").arg(s);
	}
	else if (err == ClientStream::ErrSecurityLayer)
		str = QCoreApplication::translate("@default", "Broken security layer (SASL)");
	else
		str = QCoreApplication::translate("@default", "None");
	//printf("str[%s], reconn=%d\n", str.latin1(), reconn);
	*_str = str;
	*_reconn = reconn;
}
*/