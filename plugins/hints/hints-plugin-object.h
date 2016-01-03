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

#include "plugin/plugin-object.h"

#include <QtCore/QPointer>
#include <injeqt/injeqt.h>

class ConfigurationUiHandlerRepository;
class HintsConfigurationUiHandler;
class HintManager;
class PathsProvider;

class HintsPluginObject : public PluginObject
{
	Q_OBJECT

public:
	Q_INVOKABLE explicit HintsPluginObject(QObject *parent = nullptr);
	virtual ~HintsPluginObject();

	virtual void init();
	virtual void done();

private:
	QPointer<ConfigurationUiHandlerRepository> m_configurationUiHandlerRepository;
	QPointer<HintsConfigurationUiHandler> m_hintsConfigurationUiHandler;
	QPointer<HintManager> m_hintManager;
	QPointer<PathsProvider> m_pathsProvider;

private slots:
	INJEQT_SETTER void setConfigurationUiHandlerRepository(ConfigurationUiHandlerRepository *configurationUiHandlerRepository);
	INJEQT_SETTER void setHintsConfigurationUiHandler(HintsConfigurationUiHandler *hintsConfigurationUiHandler);
	INJEQT_SETTER void setHintManager(HintManager *hintManager);
	INJEQT_SETTER void setPathsProvider(PathsProvider *pathsProvider);

};