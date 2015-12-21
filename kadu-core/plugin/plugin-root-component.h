/*
 * Copyright 2009 Wojciech Treter (juzefwt@gmail.com)
 * Copyright 2004 Adrian Smarzewski (adrian@kadu.net)
 * Copyright 2004, 2006 Marcin Ślusarz (joi@kadu.net)
 * Copyright 2007, 2008, 2009, 2010, 2011, 2013, 2014 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
 * Copyright 2014 Bartosz Brachaczek (b.brachaczek@gmail.com)
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

#include <QtCore/QtPlugin>

class PluginRepository;

/**
 * @addtogroup Plugin
 * @{
 */

/**
 * @class PluginRootComponent
 * @author Rafał 'Vogel' Malinowski
 * @short Base interface for all Kadu plugins.
 *
 * Every Kadu plugin has to have a class that inherits from PluginRootComponent. Next this class has to
 * be registered using Q_EXPORT_PLUGIN2 macro with plugin library name and class name as parameters.
 *
 * A new instance of given object is created when such plugin is loaded and then init() method is
 * called. Before plugin gets unloaded done() method is called.
 */
class PluginRootComponent
{
public:
	virtual ~PluginRootComponent() {}

	/**
	 * @author Rafał 'Vogel' Malinowski
	 * @short Method called after plugin activation.
	 * @param firstLoad true, if this is first activation of current plugin
	 *
	 * This method is called every time a plugin is activated. Implementations should do all work
	 * needed to properly run plugin, like registering protocols, notifications and do on, in this
	 * method.
	 */
	virtual bool init(PluginRepository *pluginRepository) = 0;

	/**
	 * @author Rafał 'Vogel' Malinowski
	 * @short Method called before plugin deactivation.
	 *
	 * This method is called before a plugin is deactivated. Implementations should do all work
	 * needed to properly finalize plugin, like unregistering protocols, notifications and do on,
	 * in this method. Every action run in init() has to have a counterpart in this method.
	 */
	virtual void done() = 0;

};

Q_DECLARE_INTERFACE(PluginRootComponent, "im.kadu.PluginRootComponent")

/**
 * @}
 */

#ifdef Q_EXPORT_PLUGIN2
#	undef Q_EXPORT_PLUGIN2
#endif
#define Q_EXPORT_PLUGIN2(function, plugin)
