/*
 * %kadu copyright begin%
 * Copyright 2009 Wojciech Treter (juzefwt@gmail.com)
 * Copyright 2008, 2009, 2010 Rafał Malinowski (rafal.przemyslaw.malinowski@gmail.com)
 * Copyright 2008 Michał Podsiadlik (michal@kadu.net)
 * Copyright 2009 Piotr Galiszewski (piotrgaliszewski@gmail.com)
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

#include "configuration/configuration_window.h"
#include "accounts/account.h"
#include "accounts/account_manager.h"
#include "kadu.h"
#include "config_file.h"
#include "debug.h"
#include "misc/misc.h"

#include "filedesc.h"

#include <QtCore/QTextStream>
#include <QtCore/QTimer>

#define MODULE_FILEDESC_VERSION 1.13

FileDescription *file_desc;

extern "C" KADU_EXPORT int filedesc_init()
{
	file_desc = new FileDescription();
	MainConfigurationWindow::registerUiFile(dataPath("kadu/modules/configuration/filedesc.ui"), file_desc);

	return 0;
}

extern "C" KADU_EXPORT void filedesc_close()
{
	MainConfigurationWindow::unregisterUiFile(dataPath("kadu/modules/configuration/filedesc.ui"), file_desc);
	delete file_desc;
}

// Implementation of FileDescStatusChanger class

FileDescStatusChanger::FileDescStatusChanger()
	: StatusChanger(900), disabled(true)
{
}

FileDescStatusChanger::~FileDescStatusChanger()
{
}

void FileDescStatusChanger::changeStatus(UserStatus &status)
{
	if (status.isOffline())
		return;
	if (!disabled)
		status.setDescription(title);
}

void FileDescStatusChanger::setTitle(const QString &newTitle)
{
	disabled = false;

	if (newTitle != title)
	{
		title = newTitle;
		emit statusChanged();
	}
}

void FileDescStatusChanger::disable()
{
	if (!disabled)
	{
		disabled = true;
		emit statusChanged();
	}
}

// Implementation of FileDescription class

FileDescription::FileDescription()
{
	kdebugf();

	createDefaultConfiguration();

	timer = new QTimer();
	connect(timer, SIGNAL(timeout()), this, SLOT(checkTitle()));
	timer->start(500);

	fileDescStatusChanger = new FileDescStatusChanger();
	StatusChangerManager::instance()->registerStatusChanger(fileDescStatusChanger);
}

FileDescription::~FileDescription()
{
	kdebugf();
	disconnect(timer, SIGNAL(timeout()), this, SLOT(checkTitle()));
	delete timer;

	StatusChangerManager::instance()->unregisterStatusChanger(fileDescStatusChanger);
	delete fileDescStatusChanger;
	fileDescStatusChanger = 0;
}

void FileDescription::checkTitle()
{
	Protocol *gadu = AccountManager::instance()->defaultAccount()->protocol();

	if (QFile::exists(config_file.readEntry("FileDesc", "file")))
	{
		QFile file(config_file.readEntry("FileDesc", "file"));
		if (!gadu->currentStatus().isOffline())
		{
			if (file.open(IO_ReadOnly))
			{
				QString desc = "";
				QTextStream stream(&file);
				if (!stream.atEnd())
					desc = stream.readLine();

				if ((desc != currDesc || gadu->currentStatus().description() != desc && !config_file.readBoolEntry("FileDesc", "allowOther")
					|| !gadu->currentStatus().hasDescription() && config_file.readBoolEntry("FileDesc", "forceDescr")) && !gadu->currentStatus().isOffline())
				{
					currDesc = desc;
					fileDescStatusChanger->setTitle(desc);
				}
				file.close();
			}
		}
	}
}

void FileDescription::createDefaultConfiguration()
{
	config_file.addVariable("FileDesc", "file", profilePath("description.txt"));
	config_file.addVariable("FileDesc", "forceDescr", true);
	config_file.addVariable("FileDesc", "allowOther", true);
}
