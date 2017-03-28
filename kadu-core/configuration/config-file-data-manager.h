/*
 * %kadu copyright begin%
 * Copyright 2011 Piotr Galiszewski (piotr.galiszewski@kadu.im)
 * Copyright 2010 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2008, 2009, 2010, 2011, 2013, 2014 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include "configuration-window-data-manager.h"

#include <QtCore/QPointer>
#include <injeqt/injeqt.h>

class Configuration;

class KADUAPI ConfigFileDataManager : public ConfigurationWindowDataManager
{
    Q_OBJECT

public:
    ConfigFileDataManager(QObject *parent = nullptr) : ConfigurationWindowDataManager(parent)
    {
    }

    virtual void writeEntry(const QString &section, const QString &name, const QVariant &value);
    virtual QVariant readEntry(const QString &section, const QString &name);

private:
    QPointer<Configuration> m_configuration;

private slots:
    INJEQT_SET void setConfiguration(Configuration *configuration);
};
