/*
 * %kadu copyright begin%
 * Copyright 2011 Piotr Galiszewski (piotr.galiszewski@kadu.im)
 * Copyright 2010 Tomasz Rostański (rozteck@interia.pl)
 * Copyright 2011 Piotr Dąbrowski (ultr@ultr.pl)
 * Copyright 2014 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2010, 2011, 2012, 2013, 2014 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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
#include <QtWidgets/QDialog>
#include <injeqt/injeqt.h>

class IconsManager;

class QLabel;
class QListWidget;
class QProgressBar;

/**
 * @addtogroup Gui
 * @{
 */

/**
 * @class ProgressWindow
 * @short Window used to show progress of an action to user.
 *
 * This window can be used to show progres of any action to user. Window consists of label, progress
 * bar and list displaying progress entries, similar to some installers.
 *
 * Value of label is set in constructor and cannot be changed. By default progress bar is showing busy
 * incidator. After action is finished progress bar shows 100% or 0% progress, depending if action
 * failed or not. Also alert is set on window.
 *
 * If action failed additional message box is displayed if message text is provided.
 *
 * Window can only be closed after action is finished or if its Cancellable property is set to true.
 * When window is closed before action is finished and Cancellable is true @link cancelled() @endlink
 * is emited.
 */
class KADUAPI ProgressWindow : public QDialog
{
    Q_OBJECT

    QPointer<IconsManager> m_iconsManager;

    QProgressBar *ProgressBar;
    QListWidget *TextListWidget;
    QPushButton *ShowDetailsButton;
    QPushButton *CloseButton;

    QString Label;
    bool Finished;
    bool Cancellable;

    void createGui();

private slots:
    INJEQT_SET void setIconsManager(IconsManager *iconsManager);

    void showDetailsClicked();

protected:
    virtual void closeEvent(QCloseEvent *closeEvent);
    virtual void keyPressEvent(QKeyEvent *keyEvent);

public:
    /**
     * @short Create new progress window with given label.
     * @param label label of progress window
     * @param parent parent QWidget of progress window
     */
    explicit ProgressWindow(const QString &label, QWidget *parent = nullptr);
    virtual ~ProgressWindow();

    /**
     * @short Set value of Cancellable property.
     * @param setCancellable new value of Cancellable property
     *
     * If window is Cancellable it can be closed before action is finished. Signal @link canceled() @endlink will be
     * emited in such case.
     */
    void setCancellable(bool cancellable);

public slots:
    /**
     * @short Add new progress entry to progress list widget.
     * @param entryIcon name of KaduIcon that will be displayed next to progress message
     * @param entryMessage message to add at bottom of progress list
     */
    void addProgressEntry(const QString &entryIcon, const QString &entryMessage);

    /**
     * @short Set value of progress bar.
     * @param value new value of progress bar
     * @param maximum new maximum value of progress bar
     *
     * Set both values to 0 to make progress bar show busy indicator.
     */
    void setProgressValue(int value, int maximum);

    /**
     * @short Add new progress entry to progress list widget and enable closing of this window.
     * @param ok true if action was successfull
     * @param entryIcon
     * @param entryIcon name of KaduIcon that will be displayed next to progress message
     * @param entryMessage message to add at bottom of progress list
     *
     * This method works like addProgressEntry() and additionally enables closing of this progress
     * window. Also alert is set on this window and if ok is false a message dialog with given
     * icon and message is displayed.
     */
    void progressFinished(bool ok, const QString &entryIcon, const QString &entryMessage);

signals:
    /**
     * @short Emited when window is closed before finishing its task.
     */
    void canceled();
};

/**
 * @}
 */
