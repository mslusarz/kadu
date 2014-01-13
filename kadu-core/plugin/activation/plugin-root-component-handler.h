/*
 * %kadu copyright begin%
 * Copyright 2013 Rafał Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include <QtCore/QObject>

class PluginRootComponent;

/**
 * @addtogroup Plugin
 * @{
 */

/**
 * @class PluginRootComponentHandler
 * @short RAII class for handling plugin root object.
 *
 * This class handles plugin root object. It calls its init() method in constructor and
 * done() method in destructor. done() method is called only when init() method returned
 * true.
 */
class PluginRootComponentHandler : QObject
{
	Q_OBJECT

public:
	/**
	 * @short Initialize plugin root object.
	 * @param pluginName name of plugin
	 * @param firstLoad parameter passed back to init() method of plugin root object
	 * @param pluginRootComponent root object of plugin
	 * @param parent Qt parent object
	 * @throws PluginActivationErrorException
	 *
	 * If pluginRootComponent is nullptr a PluginActivationErrorException is thrown. If not, init() method
	 * of this object is called. If its result is false, PluginActivationErrorException is thrown.
	 */
	PluginRootComponentHandler(const QString &pluginName, bool firstLoad, PluginRootComponent *pluginRootComponent, QObject *parent = nullptr) noexcept(false);

	/**
	 * @short Finalize plugin root object.
	 *
	 * If stored @see PluginRootComponent is not done() method of this object is called.
	 */
	~PluginRootComponentHandler() noexcept;

private:
	PluginRootComponent *m_pluginRootComponent;

};

/**
 * @}
 */