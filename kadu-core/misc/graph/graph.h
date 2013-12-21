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

#include "misc/graph/graph-node.h"
#include "misc/memory.h"
#include "exports.h"

#include <memory>

template<typename P, typename... SuccessorTypeTags>
class Graph
{

public:
	using Type = Graph<P, SuccessorTypeTags...>;
	using Pointer = Type*;
	using PayloadType = P;
	using NodeType = GraphNode<P, SuccessorTypeTags...>;
	using NodePointer = NodeType*;

	Graph() = default;
	Graph(const Graph &) = delete;

	Graph(Graph &&moveMe) :
			m_nodes{std::move(moveMe.m_nodes)}
	{
	}

	Graph & operator = (const Graph &) = delete;

	Graph & operator = (Graph &&moveMe)
	{
		using std::swap;
		swap(m_nodes, moveMe.m_nodes);
		return *this;
	}

	const std::vector<std::unique_ptr<NodeType>> & nodes() const
	{
		return m_nodes;
	}

	NodePointer addNode(P payload)
	{
		auto node = make_unique<NodeType>(payload);
		auto result = node.get();
		m_nodes.push_back(std::move(node));
		return result;
	}

	template<typename SuccessorTypeTag>
	bool addEdge(const P &from, const P &to)
	{
		auto nodeFrom = node(from);
		auto nodeTo = node(to);

		if (!nodeFrom || !nodeTo)
			return false;

		nodeFrom->template addSuccessor<SuccessorTypeTag>(nodeTo);
		return true;
	}

	NodePointer node(const P &payload) const
	{
		auto match = [&payload](const std::unique_ptr<NodeType> &node) { return node.get()->payload() == payload; };
		auto result = std::find_if(m_nodes.cbegin(), m_nodes.cend(), match);
		return result != m_nodes.end()
				? (*result).get()
				: nullptr;
	}

private:
	std::vector<std::unique_ptr<NodeType>> m_nodes;

};