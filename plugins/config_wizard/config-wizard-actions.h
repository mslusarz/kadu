/*
 * %kadu copyright begin%
 * Copyright 2011, 2014 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2011, 2014 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include <QtCore/QObject>
#include <QtCore/QPointer>
#include <injeqt/injeqt.h>

class ShowConfigWizardAction;

class MenuInventory;

class QAction;

class ConfigWizardActions : public QObject
{
    Q_OBJECT

public:
    Q_INVOKABLE explicit ConfigWizardActions(QObject *parent = nullptr);
    virtual ~ConfigWizardActions();

    void registerActions();
    void unregisterActions();

private:
    QPointer<MenuInventory> m_menuInventory;
    QPointer<ShowConfigWizardAction> m_showConfigWizardAction;

private slots:
    INJEQT_SET void setMenuInventory(MenuInventory *menuInventory);
    INJEQT_SET void setShowConfigWizardAction(ShowConfigWizardAction *showConfigWizardAction);
};
