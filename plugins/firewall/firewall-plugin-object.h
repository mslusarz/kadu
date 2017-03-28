/*
 * %kadu copyright begin%
 * Copyright 2015 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include <QtCore/QPointer>
#include <injeqt/injeqt.h>

class ConfigurationUiHandlerRepository;
class FirewallConfigurationUiHandler;
class FirewallMessageFilter;
class MainConfigurationWindowService;
class MessageFilterService;
class PathsProvider;

class FirewallPluginObject : public QObject
{
    Q_OBJECT
    INJEQT_TYPE_ROLE(PLUGIN)

public:
    Q_INVOKABLE explicit FirewallPluginObject(QObject *parent = nullptr);
    virtual ~FirewallPluginObject();

private:
    QPointer<ConfigurationUiHandlerRepository> m_configurationUiHandlerRepository;
    QPointer<FirewallConfigurationUiHandler> m_firewallConfigurationUiHandler;
    QPointer<FirewallMessageFilter> m_firewallMessageFilter;
    QPointer<MainConfigurationWindowService> m_mainConfigurationWindowService;
    QPointer<MessageFilterService> m_messageFilterService;
    QPointer<PathsProvider> m_pathsProvider;

private slots:
    INJEQT_SET void
    setConfigurationUiHandlerRepository(ConfigurationUiHandlerRepository *configurationUiHandlerRepository);
    INJEQT_SET void setFirewallConfigurationUiHandler(FirewallConfigurationUiHandler *firewallConfigurationUiHandler);
    INJEQT_SET void setFirewallMessageFilter(FirewallMessageFilter *firewallMessageFilter);
    INJEQT_SET void setMainConfigurationWindowService(MainConfigurationWindowService *mainConfigurationWindowService);
    INJEQT_SET void setMessageFilterService(MessageFilterService *messageFilterService);
    INJEQT_SET void setPathsProvider(PathsProvider *pathsProvider);
    INJEQT_INIT void init();
    INJEQT_DONE void done();
};
