/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CONFIG_H
#define CONFIG_H

#include <qtabdialog.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qslider.h>
#include <qvgroupbox.h>
#include <qhbox.h>
#include <qvaluelist.h>
#include "../config.h"

void loadKaduConfig(void);
void saveKaduConfig(void);

class ConfigDialog : public QTabDialog	{
	Q_OBJECT

	public:
		ConfigDialog(QWidget *parent=0, const char *name=0);

	protected:
		void setupTab1();
		void setupTab2();
		void setupTab3();
		void setupTab4();
		void setupTab5();
		void setupTab6();

		QComboBox *cb_defstatus;
		QCheckBox *b_geometry;
		QCheckBox *b_logging;
		QLineEdit *e_password;
		QLineEdit *e_uin;
		QLineEdit *e_nick;
		QHBox     *smshbox1;
		QHBox     *smshbox2;
		QCheckBox *b_smsbuildin;
		QLineEdit *e_smsapp;
		QLineEdit *e_smsconf;
		QCheckBox *b_smscustomconf;
		QCheckBox *b_autoaway;
		QLineEdit *e_autoawaytime;
		QCheckBox *b_dock;
		QCheckBox *b_private;
		QCheckBox *b_rdocked;
		QCheckBox *b_grptabs;
		QCheckBox *b_checkupdates;
		QCheckBox *b_addtodescription;
		QCheckBox *b_showhint;
		QCheckBox *b_showdesc;

		QLineEdit *e_soundprog;
		QCheckBox *b_playsound;
		QCheckBox *b_playartsdsp;
		QLineEdit *e_msgfile;
		QLineEdit *e_chatfile;
		QCheckBox *b_playchat;
		QCheckBox *b_playchatinvisible;
		QSlider *s_volume;
		QCheckBox *b_soundvolctrl;

		QCheckBox *b_emoticons;
		QComboBox *cb_emoticons_theme;
		QCheckBox *b_autosend;
		QCheckBox *b_scrolldown;
		QLineEdit *e_emoticonspath;
		QCheckBox *b_chatprune;
		QLineEdit *e_chatprunelen;
		QCheckBox *b_msgacks;
		QCheckBox *b_blinkchattitle;
		QHBox     *webhbox1;
		QCheckBox *b_defwebbrowser;
		QLineEdit *e_webbrowser;
		QCheckBox *b_ignoreanonusers;

		QVGroupBox *notifybox;
		QHBox *panebox;
		QListBox *e_notifies;
		QListBox *e_availusers;
		QCheckBox *b_notifyall;
		QCheckBox *b_notifyglobal;
		QCheckBox *b_notifydialog;
		QCheckBox *b_notifysound;
		QLineEdit *e_soundnotify;

		QCheckBox *b_dccenabled;
		QCheckBox *b_dccip;
		QVGroupBox *g_dccip;
		QLineEdit *e_dccip;
		QCheckBox *b_dccfwd;
		QVGroupBox *g_fwdprop;
		QLineEdit *e_extip;
		QLineEdit *e_extport;
		QCheckBox *b_defserver;
		QVGroupBox *g_server;
		QLineEdit *e_server;
		QVGroupBox *g_proxy;
		QCheckBox *b_useproxy;
		QLineEdit *e_proxyserver;
		QLineEdit *e_proxyport;
		QLineEdit *e_proxyuser;
		QLineEdit *e_proxypassword;

		QComboBox *cb_chatfont;
		QComboBox *cb_chatfontsize;
		QComboBox *cb_userboxfont;
		QComboBox *cb_userboxfontsize;
		QComboBox *cb_chatselect;
		QComboBox *cb_userboxselect;
		QPushButton *pb_chatcolor;
		QPushButton *pb_userboxcolor;
		QLineEdit *e_chatcolor;
		QLineEdit *e_userboxcolor;
		QHBox *chatselectfont;
		QHBox *userboxselectfont;
		QValueList<QColor> vl_userboxcolor;
		QValueList<QFont> vl_userboxfont;
		QValueList<QColor> vl_chatcolor;
		QValueList<QFont> vl_chatfont;

#ifdef HAVE_OPENSSL
		QCheckBox *b_encryption;
		QCheckBox *b_encryptmsg;
		QComboBox *cb_keyslen;
		QPushButton *pb_encryption;
#endif

	protected slots:
		void _Left();
		void _Right();
		void updateConfig();
		void chooseMsgFile();
		void chooseChatFile();
		void choosePlayerFile();
		void chooseNotifyFile();
		void chooseEmoticonsPath();
		void generateMyKeys();
		void chooseChatColorGet();
		void chooseUserboxColorGet();
		void chooseChatFontGet(int index);
		void chooseUserboxFontGet(int index);
		void chooseChatSelect(int index);
		void chooseUserboxSelect(int index);
		void chooseUserboxFontSizeGet(int index);
		void chooseChatFontSizeGet(int index);
		void chooseChatLine(const QString&);
		void chooseUserboxLine(const QString&);
		void chooseMsgTest();
		void chooseChatTest();
		void chooseNotifyTest();
		void ifDockEnabled(bool);
		void ifNotifyGlobal(bool);
		void ifNotifyAll(bool);
		void ifDccEnabled(bool);
		void ifDccIpEnabled(bool);
		void ifDefServerEnabled(bool);
		void ifUseProxyEnabled(bool);
		void onSmsBuildInCheckToogle(bool);
		void onDefWebBrowserToogle(bool);
};

#endif
