/*
 * %kadu copyright begin%
 * Copyright 2014 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2013, 2014 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include "configuration/configuration-aware-object.h"

#include <QtCore/QPointer>

class ChatWindowStorage;
class ChatWindowStorageConfiguration;
class Configuration;

class ChatWindowStorageConfigurator : private ConfigurationAwareObject
{
public:
    ChatWindowStorageConfigurator(Configuration *configuration);

    void setChatWindowStorage(ChatWindowStorage *chatWindowStorage);
    ChatWindowStorageConfiguration loadConfiguration() const;

protected:
    virtual void configurationUpdated() override;

private:
    QPointer<ChatWindowStorage> m_chatWindowStorage;
    QPointer<Configuration> m_configuration;

    void createDefaultConfiguration() const;
};
