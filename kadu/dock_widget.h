/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DOCKAPP_H
#define DOCKAPP_H

#include <qlabel.h>
#include <qtextbrowser.h>
#include <qstringlist.h>
#include <qpixmap.h>

class TrayIcon : public QWidget
{
	Q_OBJECT

	private:
		friend class TrayHint;
		QLabel* icon;
		TrayHint *hint;
		QTimer *icon_timer;
		bool blink;

	protected:
		void setPixmap(const QPixmap& pixmap);
		virtual void resizeEvent(QResizeEvent* e);
		virtual void enterEvent(QEvent* e);
		virtual void mousePressEvent(QMouseEvent*);

	public:
		TrayIcon(QWidget *parent = 0, const char *name = 0);
		~TrayIcon();
		QPoint trayPosition();
		void setType(QPixmap &pixmap);
		void connectSignals();
		void showHint(const QString&, const QString&, int index);
		void showErrorHint(const QString&);
		void reconfigHint();

	public slots:
		// Status change slots
		void dockletChange(int id);
		//Funkcja do migania koperty
		void changeIcon();
};

class TrayHint : public QWidget
{
	Q_OBJECT
	
	private:
		QTimer *hint_timer;
		QTextBrowser *hint;
		QStringList hint_list;

	public:
		TrayHint(QWidget *parent=0, const char *name = 0);
		void show_hint(const QString&, const QString&, int index);
		void restart();
		
	public slots:
		void remove_hint();

	private slots:
		void set_hint();
};

extern TrayIcon *trayicon;

#endif
