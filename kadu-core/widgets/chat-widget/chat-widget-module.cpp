/*
 * %kadu copyright begin%
 * Copyright 2014, 2015 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include "chat-widget-module.h"

#include "widgets/chat-widget/chat-widget-activation-service.h"
#include "widgets/chat-widget/chat-widget-container-handler-mapper.h"
#include "widgets/chat-widget/chat-widget-container-handler-repository.h"
#include "widgets/chat-widget/chat-widget-manager.h"
#include "widgets/chat-widget/chat-widget-message-handler-configurator.h"
#include "widgets/chat-widget/chat-widget-message-handler.h"
#include "widgets/chat-widget/chat-widget-repository-impl.h"
#include "widgets/chat-widget/chat-widget-state-persistence-service.h"

ChatWidgetModule::ChatWidgetModule()
{
    add_type<ChatWidgetActivationService>();
    add_type<ChatWidgetContainerHandlerMapper>();
    add_type<ChatWidgetContainerHandlerRepository>();
    add_type<ChatWidgetManager>();
    add_type<ChatWidgetMessageHandlerConfigurator>();
    add_type<ChatWidgetMessageHandler>();
    add_type<ChatWidgetRepositoryImpl>();
    add_type<ChatWidgetStatePersistenceService>();
}

ChatWidgetModule::~ChatWidgetModule()
{
}
