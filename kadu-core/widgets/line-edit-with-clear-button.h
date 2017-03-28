/*
 * %kadu copyright begin%
 * Copyright 2011 Piotr Galiszewski (piotr.galiszewski@kadu.im)
 * Copyright 2012 Piotr Dąbrowski (ultr@ultr.pl)
 * Copyright 2011, 2014 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2010, 2011, 2013, 2014 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include "exports.h"

#include <QtCore/QPointer>
#include <QtWidgets/QLineEdit>
#include <injeqt/injeqt.h>

class IconsManager;
class LineEditClearButton;

class KADUAPI LineEditWithClearButton : public QLineEdit
{
    Q_OBJECT

public:
    explicit LineEditWithClearButton(QWidget *parent = nullptr);
    virtual ~LineEditWithClearButton();

    bool isClearButtonVisible() const
    {
        return ClearButtonVisible;
    }
    void setClearButtonVisible(bool clearButtonVisible);

    virtual void setReadOnly(bool readonly);
    virtual void setEnabled(bool enabled);

signals:
    void cleared();

protected:
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void mouseReleaseEvent(QMouseEvent *e);
    virtual void resizeEvent(QResizeEvent *e);

private:
    QPointer<IconsManager> m_iconsManager;

    bool ClearButtonVisible;

    LineEditClearButton *ClearButton;

    bool WideEnoughForClear;
    bool ClickInClear;

    void createClearButton();
    void updateClearButton();
    bool canShowClearButton();

private slots:
    INJEQT_SET void setIconsManager(IconsManager *iconsManager);

    void updateClearButtonIcon();
};
