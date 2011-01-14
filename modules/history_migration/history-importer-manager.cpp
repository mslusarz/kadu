/*
 * %kadu copyright begin%
 * Copyright 2011 Rafał Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include "history-importer.h"

#include "history-importer-manager.h"

HistoryImporterManager * HistoryImporterManager::Instance = 0;

void HistoryImporterManager::createInstance()
{
	if (!Instance)
		Instance = new HistoryImporterManager();
}

void HistoryImporterManager::destroyInstance()
{
	delete Instance;
	Instance = 0;
}

HistoryImporterManager::HistoryImporterManager()
{
}

HistoryImporterManager::~HistoryImporterManager()
{
	foreach (HistoryImporter *importer, Importers)
		importer->canceled();
}

void HistoryImporterManager::addImporter(HistoryImporter *importer)
{
	Importers.append(importer);
	connect(importer, SIGNAL(destroyed(QObject*)), this, SLOT(importerDestroyed(QObject*)));
}

void HistoryImporterManager::removeImporter(HistoryImporter *importer)
{
	Importers.removeAll(importer);
	disconnect(importer, SIGNAL(destroyed(QObject*)), this, SLOT(importerDestroyed(QObject*)));
}

void HistoryImporterManager::importerDestroyed(QObject *importer)
{
	Importers.removeAll(static_cast<HistoryImporter *>(importer));
}
