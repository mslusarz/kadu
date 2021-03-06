/*
 * %kadu copyright begin%
 * Copyright 2014 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include "webkit-messages-view-factory.h"

#include "chat-style/engine/chat-style-renderer-factory-provider.h"
#include "core/injected-factory.h"
#include "widgets/webkit-messages-view/webkit-messages-view-handler-factory.h"
#include "widgets/webkit-messages-view/webkit-messages-view.h"

WebkitMessagesViewFactory::WebkitMessagesViewFactory(QObject *parent) : QObject{parent}
{
}

WebkitMessagesViewFactory::~WebkitMessagesViewFactory()
{
}

void WebkitMessagesViewFactory::setChatStyleRendererFactoryProvider(
    ChatStyleRendererFactoryProvider *chatStyleRendererFactoryProvider)
{
    m_chatStyleRendererFactoryProvider = chatStyleRendererFactoryProvider;
}

void WebkitMessagesViewFactory::setInjectedFactory(InjectedFactory *injectedFactory)
{
    m_injectedFactory = injectedFactory;
}

void WebkitMessagesViewFactory::setWebkitMessagesViewHandlerFactory(
    WebkitMessagesViewHandlerFactory *webkitMessagesViewHandlerFactory)
{
    m_webkitMessagesViewHandlerFactory = webkitMessagesViewHandlerFactory;
}

owned_qptr<WebkitMessagesView>
WebkitMessagesViewFactory::createWebkitMessagesView(Chat chat, bool supportTransparency, QWidget *parent)
{
    auto result = m_injectedFactory->makeOwned<WebkitMessagesView>(chat, supportTransparency, parent);
    result->setChatStyleRendererFactory(m_chatStyleRendererFactoryProvider->chatStyleRendererFactory());
    result->setWebkitMessagesViewHandlerFactory(m_webkitMessagesViewHandlerFactory);
    result->refreshView();

    connect(
        m_chatStyleRendererFactoryProvider,
        SIGNAL(chatStyleRendererFactoryChanged(std::shared_ptr<ChatStyleRendererFactory>)), result.get(),
        SLOT(setChatStyleRendererFactory(std::shared_ptr<ChatStyleRendererFactory>)));

    return result;
}
