/*
 * %kadu copyright begin%
 * Copyright 2008, 2010, 2011 Piotr Galiszewski (piotr.galiszewski@kadu.im)
 * Copyright 2010 Tomasz Rostański (rozteck@interia.pl)
 * Copyright 2011 Piotr Dąbrowski (ultr@ultr.pl)
 * Copyright 2008 Michał Podsiadlik (michal@kadu.net)
 * Copyright 2007, 2008, 2009, 2010, 2011 Rafał Malinowski (rafal.przemyslaw.malinowski@gmail.com)
 * Copyright 2007, 2008 Dawid Stawiarski (neeo@kadu.net)
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

#ifndef MERGE_BUDDIES_DIALOG_WIDGET_H
#define MERGE_BUDDIES_DIALOG_WIDGET_H

#include <QtCore/QVariant>
#include <QtGui/QDialog>

#include "buddies/buddy.h"
#include "gui/widgets/dialog/dialog-widget.h"

class SelectTalkableComboBox;

class MergeBuddiesDialogWidget : public DialogWidget
{
	Q_OBJECT

	Buddy MyBuddy;

	SelectTalkableComboBox *SelectCombo;

	virtual void createGui();

private slots:
	void selectedBuddyChanged();
	virtual void dialogAccepted();
	virtual void dialogRejected();

public:
	explicit MergeBuddiesDialogWidget(Buddy buddy, const QString &message, QWidget* parent);
	virtual ~MergeBuddiesDialogWidget();
};

#endif // MERGE_BUDDIES_DIALOG_WIDGET_H