/*
 * %kadu copyright begin%
 * Copyright 2012 Wojciech Treter (juzefwt@gmail.com)
 * Copyright 2012 Piotr Dąbrowski (ultr@ultr.pl)
 * Copyright 2012, 2013 Rafał Malinowski (rafal.przemyslaw.malinowski@gmail.com)
 * Copyright 2012 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * %kadu copyright end%
 *
 * This file is derived from part of the KDE project
 * Copyright (C) 2007, 2006 Rafael Fernández López <ereslibre@kde.org>
 * Copyright (C) 2002-2003 Matthias Kretz <kretz@kde.org>
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

#include "plugin-proxy-model.h"

#include "plugin/model/plugin-model.h"

PluginProxyModel::PluginProxyModel(QObject *parent) :
		CategorizedSortFilterProxyModel{parent}
{
	sort(0);
}

PluginProxyModel::~PluginProxyModel()
{
}

void PluginProxyModel::setFilterText(const QString &filterText)
{
	if (m_filterText == filterText)
		return;

	m_filterText = filterText;
	invalidate();
}

bool PluginProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
	Q_UNUSED(sourceParent)

	if (m_filterText.isEmpty())
		return true;

	auto index = sourceModel()->index(sourceRow, 0);
	auto entry = static_cast<PluginEntry*>(index.internalPointer());
	return entry->pluginName.contains(m_filterText, Qt::CaseInsensitive) ||
			entry->name.contains(m_filterText, Qt::CaseInsensitive) ||
			entry->description.contains(m_filterText, Qt::CaseInsensitive) ||
			entry->author.contains(m_filterText, Qt::CaseInsensitive);
}

bool PluginProxyModel::subSortLessThan(const QModelIndex &left, const QModelIndex &right) const
{
	auto leftEntry = static_cast<PluginEntry*>(left.internalPointer());
	auto rightEntry = static_cast<PluginEntry*>(right.internalPointer());
	return leftEntry->name.compare(rightEntry->name, Qt::CaseInsensitive) < 0;
}