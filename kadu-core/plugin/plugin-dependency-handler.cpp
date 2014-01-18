/*
 * %kadu copyright begin%
 * Copyright 2014 Rafał Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include "plugin-dependency-handler.h"

#include "misc/algorithm.h"
#include "misc/kadu-paths.h"
#include "plugin/dependency-graph/plugin-dependency-graph-builder.h"
#include "plugin/metadata/plugin-metadata.h"
#include "plugin/metadata/plugin-metadata-finder.h"

PluginMetadata PluginDependencyHandler::converter(WrappedIterator iterator)
{
	return iterator->second;
}

PluginDependencyHandler::PluginDependencyHandler(QObject *parent) :
		QObject(parent)
{
}

PluginDependencyHandler::~PluginDependencyHandler()
{
}

void PluginDependencyHandler::setPluginDependencyGraphBuilder(PluginDependencyGraphBuilder *pluginDependencyGraphBuilder)
{
	m_pluginDependencyGraphBuilder = pluginDependencyGraphBuilder;
}

void PluginDependencyHandler::setPluginMetadataFinder(PluginMetadataFinder *pluginMetadataFinder)
{
	m_pluginMetadataFinder = pluginMetadataFinder;
}

PluginDependencyHandler::Iterator PluginDependencyHandler::begin()
{
	return {m_allPluginMetadata.begin(), converter};
}

PluginDependencyHandler::Iterator PluginDependencyHandler::end()
{
	return {m_allPluginMetadata.end(), converter};
}

void PluginDependencyHandler::initialize()
{
	loadPluginMetadata();
	prepareDependencyGraph();
}

void PluginDependencyHandler::loadPluginMetadata()
{
	if (!m_pluginDependencyGraphBuilder || !m_pluginMetadataFinder)
		return;

	auto pluginMetatada = m_pluginMetadataFinder->readAllPluginMetadata(KaduPaths::instance()->dataPath() + QLatin1String{"plugins"});
	auto dependencyGraph = m_pluginDependencyGraphBuilder->buildGraph(pluginMetatada);
	auto pluginsInDependencyCycle = dependencyGraph.get()->findPluginsInDependencyCycle();

	std::copy_if(std::begin(pluginMetatada), std::end(pluginMetatada), std::inserter(m_allPluginMetadata, m_allPluginMetadata.begin()),
		[&pluginsInDependencyCycle](const std::map<QString, PluginMetadata>::value_type &v){ return !contains(pluginsInDependencyCycle, v.first); });
}

void PluginDependencyHandler::prepareDependencyGraph()
{
	if (!m_pluginDependencyGraphBuilder)
		return;

	m_pluginDependencyDAG = m_pluginDependencyGraphBuilder->buildGraph(m_allPluginMetadata);
}

std::set<QString> PluginDependencyHandler::pluginNames() const
{
	auto result = std::set<QString>{};
	std::transform(std::begin(m_allPluginMetadata), std::end(m_allPluginMetadata), std::inserter(result, result.begin()),
		[](const std::map<QString, PluginMetadata>::value_type &v){ return v.first; });
	return result;
}

bool PluginDependencyHandler::hasPluginMetadata(const QString &pluginName) const
{
	return contains(m_allPluginMetadata, pluginName);
}

PluginMetadata PluginDependencyHandler::pluginMetadata(const QString &pluginName) const
{
	return m_allPluginMetadata.at(pluginName);
}

QVector<QString> PluginDependencyHandler::withDependencies(const QString &pluginName) noexcept
{
	return m_pluginDependencyDAG
			? m_pluginDependencyDAG.get()->findDependencies(pluginName) << pluginName
			: QVector<QString>{};
}

QVector<QString> PluginDependencyHandler::withDependents(const QString &pluginName) noexcept
{
	return m_pluginDependencyDAG
			? m_pluginDependencyDAG.get()->findDependents(pluginName) << pluginName
			: QVector<QString>{};
}

#include "moc_plugin-dependency-handler.cpp"