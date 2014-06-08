/*
 * %kadu copyright begin%
 * Copyright 2008 Piotr Galiszewski (piotr.galiszewski@kadu.im)
 * Copyright 2009 Wojciech Treter (juzefwt@gmail.com)
 * Copyright 2007, 2008, 2009, 2010, 2011 Rafał Malinowski (rafal.przemyslaw.malinowski@gmail.com)
 * Copyright 2012 Bartosz Brachaczek (b.brachaczek@gmail.com)
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

#include "configuration/configuration.h"
#include "configuration/deprecated-configuration-api.h"
#include "misc/paths-provider.h"
#include "kadu-application.h"

#include "autostatus-configuration.h"

AutostatusConfiguration::AutostatusConfiguration()
{
	configurationUpdated();
}

void AutostatusConfiguration::configurationUpdated()
{
	AutoTime = KaduApplication::instance()->configuration()->deprecatedApi()->readNumEntry("PowerKadu", "autostatus_time", 10);
	AutoStatus = KaduApplication::instance()->configuration()->deprecatedApi()->readNumEntry("PowerKadu", "autoStatus");
	StatusFilePath = KaduApplication::instance()->configuration()->deprecatedApi()->readEntry("PowerKadu", "status_file_path", PathsProvider::instance()->profilePath() + QLatin1String("autostatus.list"));
}
