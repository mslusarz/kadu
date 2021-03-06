/*
 * %kadu copyright begin%
 * Copyright 2012 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include <QtCore/QList>
#include <QtXml/QDomText>

#include "dom-text-regexp-visitor.h"

DomTextRegexpVisitor::DomTextRegexpVisitor(QRegExp regExp) : RegExp(regExp)
{
}

DomTextRegexpVisitor::~DomTextRegexpVisitor()
{
}

QDomText DomTextRegexpVisitor::expandFirstMatch(QDomText textNode) const
{
    auto text = textNode.nodeValue();
    auto index = RegExp.indexIn(text);
    if (index < 0)
        return QDomText();

    auto length = RegExp.matchedLength();

    auto afterMatch = textNode.splitText(index + length);
    textNode.setNodeValue(textNode.nodeValue().mid(0, index));

    auto newNodes = matchToDomNodes(textNode.ownerDocument(), RegExp);
    for (auto newNode : newNodes)
        textNode.parentNode().insertBefore(newNode, afterMatch);

    return afterMatch;
}

QDomNode DomTextRegexpVisitor::visit(QDomText textNode) const
{
    auto result = textNode;
    while (!textNode.isNull())
    {
        result = textNode;
        textNode = expandFirstMatch(textNode);
    }

    return result;
}

QDomNode DomTextRegexpVisitor::beginVisit(QDomElement elementNode) const
{
    return elementNode;
}

QDomNode DomTextRegexpVisitor::endVisit(QDomElement elementNode) const
{
    return elementNode.nextSibling();
}
