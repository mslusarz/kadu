/*
 * %kadu copyright begin%
 * Copyright 2012, 2013 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include "standard-url-dom-visitor-provider.h"

#include "dom/ignore-links-dom-visitor.h"
#include "misc/memory.h"
#include "url-handlers/standard-url-expander-configurator.h"
#include "url-handlers/standard-url-expander.h"

StandardUrlDomVisitorProvider::StandardUrlDomVisitorProvider(Configuration *configuration)
        : m_ignoreLinks{std::make_unique<StandardUrlExpander>(QRegExp{"\\b(http://|https://|www\\.|ftp://)([^\\s\\\\]*)"})},
          m_configurator{configuration}
{
    m_configurator.setStandardUrlExpander(static_cast<StandardUrlExpander *>(m_ignoreLinks.visitor()));
}

StandardUrlDomVisitorProvider::~StandardUrlDomVisitorProvider()
{
}

const DomVisitor *StandardUrlDomVisitorProvider::provide() const
{
    return &m_ignoreLinks;
}
