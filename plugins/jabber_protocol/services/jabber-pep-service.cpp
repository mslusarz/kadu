/* *
 * %kadu copyright begin%
 * Copyright 2010, 2011 Piotr Galiszewski (piotr.galiszewski@kadu.im)
 * Copyright 2010 Wojciech Treter (juzefwt@gmail.com)
 * Copyright 2011 Rafał Malinowski (rafal.przemyslaw.malinowski@gmail.com)
 * Copyright 2010, 2011 Bartosz Brachaczek (b.brachaczek@gmail.com)
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

#include <QtCore/QtDebug>

#include "iris/xmpp_tasks.h"
#include "iris/xmpp_xmlcommon.h"
#include "tasks/pep-get-task.h"
#include "tasks/pep-publish-task.h"
#include "tasks/pep-retract-task.h"

#include "jabber-protocol.h"

#include "jabber-pep-service.h"

// TODO: Get affiliations upon startup, and only create nodes based on that.
// (subscriptions is not accurate, since one doesn't subscribe to the
// avatar data node)

JabberPepService::JabberPepService(XMPP::JabberProtocol *protocol) :
		QObject(protocol), ParentProtocol(protocol), XmppClient(protocol->xmppClient()), Enabled(false)
{
	if (XmppClient)
		connect(XmppClient.data(), SIGNAL(messageReceived(const Message &)), SLOT(messageReceived(const Message &)));
}

JabberPepService::~JabberPepService()
{
}

XMPP::Client * JabberPepService::xmppClient() const
{
	return XmppClient.data();
}

void JabberPepService::setEnabled(bool enabled)
{
	if (!XmppClient || Enabled == enabled)
		return;

	Enabled = enabled;

	// Publish support
	if (Enabled && !XmppClient.data()->extensions().contains("ep"))
	{
		QStringList pepNodes;
		pepNodes += "http://www.xmpp.org/extensions/xep-0084.html#ns-data";
		pepNodes += "http://www.xmpp.org/extensions/xep-0084.html#ns-metadata";
		XmppClient.data()->addExtension("ep", XMPP::Features(pepNodes));
	}
	else if (!Enabled && XmppClient.data()->extensions().contains("ep"))
		XmppClient.data()->removeExtension("ep");
}

void JabberPepService::publish(const QString &node, const XMPP::PubSubItem &it, Access access)
{
	if (!Enabled || !XmppClient)
		return;

	PEPPublishTask *tp = new PEPPublishTask(XmppClient.data()->rootTask(), node, it, access);
	connect(tp, SIGNAL(finished()), SLOT(publishFinished()));
	tp->go(true);
}


void JabberPepService::retract(const QString &node, const QString &id)
{
	if (!Enabled || !XmppClient)
		return;

	PEPRetractTask* tp = new PEPRetractTask(XmppClient.data()->rootTask(), node, id);
	// FIXME: add notification of success/failure
	tp->go(true);
}

void JabberPepService::publishFinished()
{
	PEPPublishTask *task = qobject_cast<PEPPublishTask *>(sender());
	if (!task)
		return;

	if (task->success())
		emit publishSuccess(task->node(), task->item());
	else
		emit publishError(task->node(), task->item());
}

void JabberPepService::get(const XMPP::Jid &jid, const QString &node, const QString &id)
{
	if (!Enabled || !XmppClient)
		return;

	PEPGetTask* g = new PEPGetTask(XmppClient.data()->rootTask(), jid.bare(), node, id);
	connect(g, SIGNAL(finished()), SLOT(getFinished()));
	g->go(true);
}

void JabberPepService::messageReceived(const Message &message)
{
	XMPP::Jid from = message.from();
	QString pubsubNode = message.pubsubNode();

	foreach (const XMPP::PubSubRetraction &item, message.pubsubRetractions())
		emit itemRetracted(from, pubsubNode, item);
	foreach (const XMPP::PubSubItem &item, message.pubsubItems())
		emit itemPublished(from, pubsubNode, item);
}

void JabberPepService::getFinished()
{
	PEPGetTask *task = qobject_cast<PEPGetTask *>(sender());
	if (!task)
		return;

	if (task->success())
		// Act as if the item was published. This is a convenience
		// implementation, probably should be changed later.
		if (!task->items().isEmpty())
			emit itemPublished(task->jid(),task->node(),task->items().at(0));
}