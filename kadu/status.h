/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef STATUS_H
#define STATUS_H

#include <qtimer.h>

#define GG_STATUS_INVISIBLE2 0x0009 /* g�upy... */

extern int gg_statuses[];
extern const char *statustext[];

bool ifStatusWithDescription(int status);
int getActualStatus();
int statusGGToStatusNr(int);

class AutoStatusTimer : public QTimer
{
	Q_OBJECT

	private slots:
		void onTimeout();

	public:
		AutoStatusTimer(QObject *parent = 0);

	private:
		int length_buffor;
};

class AutoAwayTimer : private QTimer
{
	Q_OBJECT

	private slots:
		void onTimeout();

	public:
		static void on(){
			if(autoaway_object==NULL)
				autoaway_object=new AutoAwayTimer();
		};
		static void off(){
			if(autoaway_object!=NULL){
				delete autoaway_object;
				autoaway_object=NULL;
				}
			};

	protected:
		bool eventFilter(QObject *, QEvent *);

	private:
    AutoAwayTimer(QObject *parent = 0);
    static AutoAwayTimer* autoaway_object;
		bool autoawayed;
		int beforeAutoAway;
};

#endif
