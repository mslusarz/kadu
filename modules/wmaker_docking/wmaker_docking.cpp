/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "wmaker_docking.h"

#include <qapplication.h>
#include <qtooltip.h>
#include <QtCore/QObject>

#include "../docking/docking.h"
#include "debug.h"
#include "config_file.h"
#include "kadu.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

/**
 * @ingroup wmaker_docking
 * @{
 */
extern Time qt_x_time;

extern "C" int wmaker_docking_init()
{
	wmaker_tray_icon = new WMakerTrayIcon(NULL, "wmaker_tray_icon");
	return 0;
}

extern "C" void wmaker_docking_close()
{
	delete wmaker_tray_icon;
	wmaker_tray_icon = NULL;
}

WMakerTrayIcon::WMakerTrayIcon(QWidget *parent, const char *name)
	: QLabel(parent, name, WMouseNoMask | WRepaintNoErase | WType_TopLevel | WStyle_Customize | WStyle_NoBorder | WStyle_StaysOnTop),
	WMakerMasterWidget(0)
{
	kdebugf();
	setBackgroundMode(X11ParentRelative);
	QPixmap pix = docking_manager->defaultPixmap();
	setMinimumSize(pix.size());
	QLabel::setPixmap(pix);
	resize(pix.size());
	setMouseTracking(true);
	update();

	Display *dsp = x11Display();
	WId win = winId();

	WMakerMasterWidget=new QWidget(0,"WMakerMasterWidget");
	WMakerMasterWidget->setGeometry(-10,-10,0,0);

	int r;
	int data = 1;
	r = XInternAtom(dsp, "KWM_DOCKWINDOW", false);
	XChangeProperty(dsp, win, r, r, 32, 0, (uchar *)&data, 1);
	r = XInternAtom(dsp, "_KDE_NET_WM_SYSTEM_TRAY_WINDOW_FOR", false);
	XChangeProperty(dsp, win, r, XA_WINDOW, 32, 0, (uchar *)&data, 1);

			
	WId w_id = WMakerMasterWidget->winId();
	XWMHints *hints;
	hints = XGetWMHints(dsp, w_id);
	hints->icon_window = win;
	hints->window_group = w_id;
	hints->flags |= WindowGroupHint | IconWindowHint;
	XSetWMHints(dsp, w_id, hints);
	XFree( hints );

	connect(docking_manager, SIGNAL(trayPixmapChanged(const QPixmap&, const QString &)), this, SLOT(setTrayPixmap(const QPixmap&, const QString &)));
	connect(docking_manager, SIGNAL(trayTooltipChanged(const QString&)), this, SLOT(setTrayTooltip(const QString&)));
	connect(docking_manager, SIGNAL(searchingForTrayPosition(QPoint&)), this, SLOT(findTrayPosition(QPoint&)));
	connect(docking_manager, SIGNAL(trayMovieChanged(const QMovie &)), this, SLOT(setTrayMovie(const QMovie &)));

	docking_manager->setDocked(true);

	show();
	kdebugf2();
}

WMakerTrayIcon::~WMakerTrayIcon()
{
	kdebugf();
	disconnect(docking_manager, SIGNAL(trayMovieChanged(const QMovie &)), this, SLOT(setTrayMovie(const QMovie &)));
	disconnect(docking_manager, SIGNAL(trayPixmapChanged(const QPixmap&, const QString &)), this, SLOT(setTrayPixmap(const QPixmap&, const QString &)));
	disconnect(docking_manager, SIGNAL(trayTooltipChanged(const QString&)), this, SLOT(setTrayTooltip(const QString&)));
	disconnect(docking_manager, SIGNAL(searchingForTrayPosition(QPoint&)), this, SLOT(findTrayPosition(QPoint&)));
	delete WMakerMasterWidget;
	docking_manager->setDocked(false);
	kdebugf2();
}

void WMakerTrayIcon::findTrayPosition(QPoint& pos)
{
	kdebugf();
	pos = mapToGlobal(QPoint(0,0));
}

void WMakerTrayIcon::show()
{
	QLabel::show();
	WMakerMasterWidget->show();
}

void WMakerTrayIcon::setTrayPixmap(const QPixmap& pixmap, const QString &/*iconName*/)
{
	kdebugf();
	QLabel::setPixmap(pixmap);
	WMakerMasterWidget->setIcon(pixmap);
	repaint();
	kdebugf2();
}

void WMakerTrayIcon::setTrayMovie(const QMovie &movie)
{
	QLabel::setMovie(movie);
	repaint();
}

void WMakerTrayIcon::setTrayTooltip(const QString& tooltip)
{
	kdebugf();
	QToolTip::add(this,tooltip);
	QToolTip::add(WMakerMasterWidget,tooltip);
	kdebugf2();
}

void WMakerTrayIcon::enterEvent(QEvent* e)
{
	kdebugf();
	if (!qApp->focusWidget())
	{
		XEvent ev;
		memset(&ev, 0, sizeof(ev));
		ev.xfocus.display = qt_xdisplay();
		ev.xfocus.type = FocusIn;
		ev.xfocus.window = winId();
		ev.xfocus.mode = NotifyNormal;
		ev.xfocus.detail = NotifyAncestor;
		Time time = qt_x_time;
		qt_x_time = 1;
		qApp->x11ProcessEvent( &ev );
		qt_x_time = time;
	}
	QWidget::enterEvent(e);
	kdebugf2();
}

void WMakerTrayIcon::mousePressEvent(QMouseEvent * e)
{
	kdebugf();
	docking_manager->trayMousePressEvent(e);
	kdebugf2();
}

WMakerTrayIcon* wmaker_tray_icon = NULL;

/** @} */

