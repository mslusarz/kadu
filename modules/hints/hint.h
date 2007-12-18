#ifndef HINT_H
#define HINT_H

#include <qcolor.h>
#include <qevent.h>
#include <qfont.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qmap.h>
#include <qobject.h>
#include <qpair.h>
#include <qpixmap.h>
#include <qstring.h>
#include <qwidget.h>

#include "configuration_aware_object.h"
#include "gadu.h"

class Notification;

class Hint : public QWidget, ConfigurationAwareObject
{
	Q_OBJECT

	QVBoxLayout *vbox;

	QHBoxLayout *labels;
	QHBoxLayout *callbacksBox;

	QLabel *icon;
	QLabel *label;
	QColor bcolor; //kolor t�a
	unsigned int secs;
	unsigned int startSecs;

	Notification *notification;
	QStringList details;

	bool haveCallbacks;

	void createLabels(const QPixmap &pixmap);
	void updateText();

	void resetTimeout();

private slots:
	void notificationClosed();

protected:
	virtual void mouseReleaseEvent(QMouseEvent * event);
	virtual void enterEvent(QEvent *event);
	virtual void leaveEvent(QEvent *event);

	virtual void configurationUpdated();

public:
	Hint(QWidget *parent, Notification *notification);
	virtual ~Hint();

	void getData(QString &text, QPixmap &pixmap, unsigned int &timeout, QFont &font, QColor &fgcolor, QColor &bgcolor);
	bool requireManualClosing();
	bool isDeprecated();

	void addDetail(const QString &detail);

	bool hasUsers() const;
	const UserListElements & getUsers() const;

public slots:
	/**
		min�a sekunda, zmniejsza licznik pozosta�ych sekund,
		zwraca true je�eli jeszcze pozosta� czas
		false, gdy czas si� sko�czy�
	**/
	void nextSecond();

	void acceptNotification();
	void discardNotification();

signals:
	void leftButtonClicked(Hint *hint);
	void rightButtonClicked(Hint *hint);
	void midButtonClicked(Hint *hint);
	void closing(Hint *hint);
	void updated(Hint *hint);
};

#endif
