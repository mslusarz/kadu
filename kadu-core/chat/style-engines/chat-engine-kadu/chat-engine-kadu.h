/*
 * %kadu copyright begin%
 * Copyright 2009, 2010, 2010, 2011 Piotr Galiszewski (piotr.galiszewski@kadu.im)
 * Copyright 2011 Piotr Dąbrowski (ultr@ultr.pl)
 * Copyright 2009, 2010, 2011, 2012 Rafał Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#ifndef CHAT_ENGINE_KADU_H
#define CHAT_ENGINE_KADU_H

#include <QtCore/QObject>
#include <QtCore/QSharedPointer>

#include "chat/style-engines/chat-engine-kadu/kadu-chat-syntax.h"

#include "chat/style-engines/chat-style-engine.h"

class Preview;
class SyntaxList;

class KaduChatStyleEngine : public QObject, public ChatStyleEngine
{
	Q_OBJECT

	QSharedPointer<SyntaxList> syntaxList;

	KaduChatSyntax CurrentChatSyntax;
	QString jsCode;

	QString formatMessage(MessageRenderInfo *message, MessageRenderInfo *after);
	void repaintMessages(HtmlMessagesRenderer *page);
	QString scriptsAtEnd(const QString &html);

private slots:
	void chatSyntaxFixup(QString &syntax);
	void chatFixup(QString &syntax);
	void validateStyleName(const QString &name, bool &valid);
	void syntaxAdded(const QString &syntaxName);

public:
	explicit KaduChatStyleEngine(QObject *parent = 0);
	virtual ~KaduChatStyleEngine();
	virtual bool supportVariants() { return false; }
	virtual QString isStyleValid(QString styleName);
	virtual bool styleUsesTransparencyByDefault(QString styleName)
	{
		Q_UNUSED(styleName)
		return false;
	}

	virtual void clearMessages(HtmlMessagesRenderer *renderer);
	virtual void appendMessages(HtmlMessagesRenderer *renderer, const QList<MessageRenderInfo *> &messages);
	virtual void appendMessage(HtmlMessagesRenderer *renderer, MessageRenderInfo *message);
	virtual void pruneMessage(HtmlMessagesRenderer *renderer);
	virtual void refreshView(HtmlMessagesRenderer *renderer, bool useTransparency = false);
	virtual void messageStatusChanged(HtmlMessagesRenderer *renderer, Message message, MessageStatus status);
	virtual void contactActivityChanged(HtmlMessagesRenderer *renderer, ChatStateService::State state, const QString &message, const QString &name);
	virtual void chatImageAvailable(HtmlMessagesRenderer *renderer, const ChatImageKey &imageKey, const QString &fileName);

	virtual void prepareStylePreview(Preview *preview, QString styleName, QString variantName);

	virtual void configurationUpdated();

	virtual void loadStyle(const QString &styleName, const QString &variantName);

};

#endif // CHAT_ENGINE_KADU_H
