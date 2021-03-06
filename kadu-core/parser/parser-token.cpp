/*
 * %kadu copyright begin%
 * Copyright 2011 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2011 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include <QtCore/QByteArray>

#include "parser-token.h"

ParserToken::ParserToken() : Type(PT_STRING), IsContentEncoded(false)
{
}

void ParserToken::setType(ParserTokenType type)
{
    Type = type;
}

QString ParserToken::decodedContent() const
{
    if (!IsContentEncoded)
        return Content;

    return QString::fromUtf8(QByteArray::fromPercentEncoding(Content.toUtf8()));
}

void ParserToken::setContent(const QString &content)
{
    Content = content;
    IsContentEncoded = false;
}

void ParserToken::encodeContent(const QByteArray &exclude, const QByteArray &include)
{
    if (IsContentEncoded || Content.isEmpty())
        return;

    Content = QString::fromUtf8(Content.toUtf8().toPercentEncoding(exclude, include));
    IsContentEncoded = true;
}
