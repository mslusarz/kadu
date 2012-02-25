/*
 * %kadu copyright begin%
 * Copyright 2008, 2009, 2010, 2010, 2011 Piotr Galiszewski (piotr.galiszewski@kadu.im)
 * Copyright 2009, 2011 Wojciech Treter (juzefwt@gmail.com)
 * Copyright 2008 Tomasz Rostański (rozteck@interia.pl)
 * Copyright 2010, 2011 Piotr Dąbrowski (ultr@ultr.pl)
 * Copyright 2008, 2009 Michał Podsiadlik (michal@kadu.net)
 * Copyright 2009 Bartłomiej Zimoń (uzi18@o2.pl)
 * Copyright 2007, 2008, 2009, 2009, 2010, 2011 Rafał Malinowski (rafal.przemyslaw.malinowski@gmail.com)
 * Copyright 2010, 2011 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2007 Dawid Stawiarski (neeo@kadu.net)
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

#include <QtCore/QDateTime>
#include <QtCore/QTimer>
#include <QtGui/QApplication>
#include <QtGui/QCloseEvent>
#include <QtGui/QDesktopWidget>
#include <QtGui/QVBoxLayout>

#include "chat/chat-details.h"
#include "chat/type/chat-type.h"
#include "configuration/configuration-file.h"
#include "contacts/contact-set.h"
#include "gui/widgets/chat-widget-manager.h"
#include "gui/widgets/chat-widget.h"
#include "gui/widgets/custom-input.h"
#include "gui/windows/message-dialog.h"
#include "message/message-manager.h"

#include "misc/misc.h"
#include "activate.h"
#include "debug.h"

#include "chat-window.h"
#include <storage/custom-properties.h>

ChatWindow::ChatWindow(ChatWidget *chatWidget, QWidget *parent) :
		QWidget(parent), DesktopAwareObject(this), currentChatWidget(chatWidget),
		title_timer(new QTimer(this)), showNewMessagesNum(false), blinkChatTitle(true)
{
	kdebugf();

	setProperty("ownWindowIcon", true);
	setWindowRole("kadu-chat");
	if (chatWidget && chatWidget->chat().details() && chatWidget->chat().details()->type())
		setWindowRole(chatWidget->chat().details()->type()->windowRole());

#ifdef Q_OS_MAC
	setAttribute(Qt::WA_MacBrushedMetal);
#endif
	setAttribute(Qt::WA_DeleteOnClose);

	currentChatWidget->setParent(this);
	currentChatWidget->show();
	currentChatWidget->edit()->setFocus();

	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->addWidget(currentChatWidget);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(0);

	kaduRestoreGeometry();
	updateTitle();
	updateIcon();

	configurationUpdated();

	connect(currentChatWidget, SIGNAL(closed()), this, SLOT(close()));
	connect(currentChatWidget, SIGNAL(iconChanged()), this, SLOT(updateIcon()));
	connect(currentChatWidget, SIGNAL(titleChanged(ChatWidget *, const QString &)), this, SLOT(updateTitle()));
	connect(title_timer, SIGNAL(timeout()), this, SLOT(blinkTitle()));
}

ChatWindow::~ChatWindow()
{
	kaduStoreGeometry();
}

void ChatWindow::configurationUpdated()
{
	triggerCompositingStateChanged();

	showNewMessagesNum = config_file.readBoolEntry("Chat", "NewMessagesInChatTitle", false);
	blinkChatTitle = config_file.readBoolEntry("Chat", "BlinkChatTitle", true);

	if (currentChatWidget->chat().unreadMessagesCount())
		blinkTitle();
}

void ChatWindow::compositingEnabled()
{
	if (config_file.readBoolEntry("Chat", "UseTransparency", false))
	{
		setAutoFillBackground(false);
		setAttribute(Qt::WA_TranslucentBackground, true);
	}
	else
		compositingDisabled();
}

void ChatWindow::compositingDisabled()
{
	setAttribute(Qt::WA_TranslucentBackground, false);
	setAttribute(Qt::WA_NoSystemBackground, false);
	setAutoFillBackground(true);
}

void ChatWindow::setDefaultGeometry()
{
	QSize size(0, 400);
	int x, y;
	x = pos().x();
	y = pos().y();
	if (currentChatWidget->chat().contacts().count() > 1)
		size.setWidth(550);
	else
		size.setWidth(400);

	QDesktopWidget *desk = qApp->desktop();

	if ((size.width() + x) > desk->width())
		x = desk->width() - size.width() - 50;
	if ((size.height() + y) > desk->height())
		y = desk->height() - size.height() - 50;

	if (x < 50) x = 50;
	if (y < 50) y = 50;

	move(x, y);
	resize(size);
}

void ChatWindow::kaduRestoreGeometry()
{
	QRect windowGeometry = stringToRect(currentChatWidget->chat().data()->customProperties()->property("chat-geometry:WindowGeometry", QString()).toString());

	if (windowGeometry.isValid())
	{
		setWindowGeometry(this, windowGeometry);
		currentChatWidget->setGeometry(windowGeometry);
		currentChatWidget->kaduRestoreGeometry();
	}
	else
		setDefaultGeometry();
}

void ChatWindow::kaduStoreGeometry()
{
	currentChatWidget->kaduStoreGeometry();

	currentChatWidget->chat().data()->customProperties()->addProperty("chat-geometry:WindowGeometry", rectToString(geometry()), CustomProperties::Storable);
}

void ChatWindow::closeEvent(QCloseEvent *e)
{
	kdebugf();

	if (config_file.readBoolEntry("Chat", "ChatCloseTimer"))
	{
		unsigned int period = config_file.readUnsignedNumEntry("Chat",
			"ChatCloseTimerPeriod", 2);

		if (QDateTime::currentDateTime() < currentChatWidget->lastReceivedMessageTime().addSecs(period))
		{
			if (!MessageDialog::ask(KaduIcon("dialog-question"), tr("Kadu"), tr("New message received, close window anyway?")))
			{
				e->ignore();
				return;
			}
		}
	}

 	QWidget::closeEvent(e);
}

void ChatWindow::updateIcon()
{
	setWindowIcon(currentChatWidget->icon());
}

void ChatWindow::updateTitle()
{
	setWindowTitle(currentChatWidget->title());

	// TODO 0.10.0: is that really needed here? this method is called only on chat widget title change
	if (showNewMessagesNum && currentChatWidget->chat().unreadMessagesCount()) // if we don't have new messages or don't want them to be shown
		showNewMessagesNumInTitle();
}

void ChatWindow::blinkTitle()
{
 	if (!_isActiveWindow(this))
  	{
		if (!windowTitle().contains(currentChatWidget->title()) || !blinkChatTitle)
		{
  			if (!showNewMessagesNum) // if we don't show number od new messages waiting
  				setWindowTitle(currentChatWidget->title());
  			else
				showNewMessagesNumInTitle();
		}
		else
			setWindowTitle(QString(currentChatWidget->title().length() + 5, ' '));

		if (blinkChatTitle) // timer will not be started, if configuration option was changed
		{
			title_timer->setSingleShot(true);
			title_timer->start(500);
		}
	}
}

void ChatWindow::showNewMessagesNumInTitle()
{
	if (!_isActiveWindow(this))
		setWindowTitle('[' + QString::number(currentChatWidget->chat().unreadMessagesCount()) + "] " + currentChatWidget->title());
}

void ChatWindow::changeEvent(QEvent *event)
{
	QWidget::changeEvent(event);
	if (event->type() == QEvent::ActivationChange)
	{
		kdebugf();
		if (_isActiveWindow(this))
		{

			MessageManager::instance()->markAllMessagesAsRead(currentChatWidget->chat());
			setWindowTitle(currentChatWidget->title());
			title_timer->stop();
		}
		kdebugf2();
	}
}

void ChatWindow::setWindowTitle(const QString &title)
{
	// qt treats [*] as 'modified placeholder'
	// we escape each [*] with double [*][*] so it gets properly handled
	QString escaped = title;
	QWidget::setWindowTitle(escaped.replace(QLatin1String("[*]"), QLatin1String("[*][*]")));
}

void ChatWindow::activateChatWidget(ChatWidget *chatWidget)
{
	Q_UNUSED(chatWidget)
	Q_ASSERT(chatWidget == currentChatWidget);

	// we can be embeded in other window...
	_activateWindow(window());
}

void ChatWindow::alertChatWidget(ChatWidget *chatWidget)
{
	Q_UNUSED(chatWidget)
	Q_ASSERT(chatWidget == currentChatWidget);

	if (isChatWidgetActive(chatWidget))
	{
		MessageManager::instance()->markAllMessagesAsRead(currentChatWidget->chat());
		return;
	}

	qApp->alert(this); // TODO: make notifier from this

	if (blinkChatTitle)
	{
		if (!title_timer->isActive())
			blinkTitle(); // blinking is able to show new messages also...
	}
	else if (showNewMessagesNum) // ... so we check this condition as 'else'
		showNewMessagesNumInTitle();
}

void ChatWindow::closeChatWidget(ChatWidget *chatWidget)
{
	Q_UNUSED(chatWidget)
	Q_ASSERT(chatWidget == currentChatWidget);

	close();
}

bool ChatWindow::isChatWidgetActive(ChatWidget *chatWidget)
{
	Q_UNUSED(chatWidget)
	Q_ASSERT(chatWidget == currentChatWidget);

	return _isWindowActiveOrFullyVisible(this);
}

