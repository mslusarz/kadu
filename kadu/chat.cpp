/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qpixmap.h>
#include <qpushbutton.h>
#include <qregexp.h>
#include <qlayout.h>
#include <qaccel.h>
#include <qtimer.h>
#include <qcursor.h>
#include <qprocess.h>
#include <qmessagebox.h>
#include <qfileinfo.h>
#include <math.h>
#include <qsplitter.h>
#include <qhbox.h>
#include <qvbox.h>

//
#include "kadu.h"
#include "pixmaps.h"
#include "chat.h"
#include "search.h"
#include "history.h"
#include "misc.h"
#include "emoticons.h"
#ifdef HAVE_OPENSSL
extern "C"
{
#include "simlite.h"
};
#endif
//

KaduTextBrowser::KaduTextBrowser(QWidget *parent, const char *name)
	: QTextBrowser(parent, name) {

}

void KaduTextBrowser::setSource(const QString &name) {
}

Chat::Chat(UinsList uins, QWidget *parent, const char *name)
 : QWidget(parent, name, Qt::WDestructiveClose), uins(uins) {
	int i;
	struct chats chat;
	QValueList<int> sizes;

	iconsel_ptr = NULL;
	autosend_enabled = false;

	title_timer = new QTimer(this);
	connect(title_timer,SIGNAL(timeout()),this,SLOT(changeTitle()));
  
	/* register us in the chats registry... */
	chat.uins = uins;
	chat.ptr = this;
	chats.append(chat);
	index = chats.count() - 1;

	QSplitter *split1, *split2;

	split1 = new QSplitter(Qt::Vertical, this);

	if (uins.count() > 1) {
		split2 = new QSplitter(Qt::Horizontal, split1);
		body = new KaduTextBrowser(split2);
		}
	else 
		body = new KaduTextBrowser(split1);
	body->setFont(config.fonts.chat);
	QObject::connect(body, SIGNAL(linkClicked(const QString &)), this, SLOT(hyperlinkClicked(const QString &)));

	QPoint pos = QCursor::pos();
	
	if (uins.count() > 1) {
		setGeometry((pos.x() + 550) / 2, (pos.y() + 400) / 2, 550, 400);
		userbox = new UserBox(split2);
		userbox->setPaletteBackgroundColor(config.colors.userboxBg);
		userbox->setPaletteForegroundColor(config.colors.userboxFg);
		userbox->QListBox::setFont(config.fonts.userbox);

		for (i = 0; i < uins.count(); i++)
			userbox->addUser(userlist.byUin(uins[i]).altnick);
		userbox->refresh();
		sizes.append(3);
		sizes.append(1);
		split2->setSizes(sizes);
		}
	else {
		setGeometry((pos.x() + 400) / 2, (pos.y() + 400) / 2, 400, 400);
		userbox = NULL;
		}
		
	QVBox *downpart = new QVBox(split1);
	QHBox *edtbuttontray = new QHBox(downpart);

	QLabel *edt = new QLabel(edtbuttontray);
	edt->setText(i18n("Edit window:"));
	QToolTip::add(edt, i18n("This is where you type in the text to be sent"));

	buttontray = new QHBox(edtbuttontray);
	autosend = new QPushButton(buttontray);
	autosend->setPixmap(loadIcon("key_enter.png"));
	autosend->setToggleButton(true);
	QToolTip::add(autosend, i18n("Enter key sends message"));
	if (config.autosend) {
		autosend->setOn(true);
		autosend_enabled = true;
		}

	lockscroll = new QPushButton(buttontray);
	lockscroll->setPixmap(QPixmap((const char**)scroll_lock));
	lockscroll->setToggleButton(true);
	QToolTip::add(lockscroll, i18n("Blocks scrolling"));

#ifdef HAVE_OPENSSL
	encryption = new QPushButton(buttontray);
	connect(encryption, SIGNAL(clicked()), this, SLOT(regEncryptSend()));

	QString keyfile_path;
	keyfile_path.append(ggPath("keys/"));
	keyfile_path.append(QString::number(uins[0]));
	keyfile_path.append(".pem");
	QFileInfo keyfile(keyfile_path);
	bool encryption_possible =
		(keyfile.permission(QFileInfo::ReadUser) && uins.count() == 1);

	setupEncryptButton(config.encryption && encryption_possible);
	
	encryption->setEnabled(encryption_possible);	
#endif
	
	QPushButton *clearchat= new QPushButton(buttontray);
	clearchat->setPixmap(loadIcon("eraser.png"));
	QToolTip::add(clearchat, i18n("Clear messages in chat window"));

	iconsel = new QPushButton(buttontray);
	iconsel->setPixmap(loadIcon("icons.png"));
	if (config.emoticons)
		QToolTip::add(iconsel, i18n("Insert emoticon"));
	else {
		QToolTip::add(iconsel, i18n("Insert emoticon - enable in configuration"));
		iconsel->setEnabled(false);
		}

	QPushButton *history = new QPushButton(buttontray);
	history->setPixmap(QPixmap((const char**)history_xpm));
	QToolTip::add(history, i18n("Show history"));

	QPushButton *whois = new QPushButton(buttontray);
	QPixmap p_whois;
	p_whois = loadIcon("viewmag.png");

	whois->setPixmap(p_whois);
	QToolTip::add(whois, i18n("Lookup user info"));

	edtbuttontray->setStretchFactor(edt, 50);
	edtbuttontray->setStretchFactor(buttontray, 1);

	edit = new CustomInput(downpart, this);
	edit->setMinimumHeight(100);
	edit->setWordWrap(QMultiLineEdit::WidgetWidth);
	edit->setFont(config.fonts.chat);

	QHBox *btnpart = new QHBox(downpart);

	QHBox *fillerbox = new QHBox(btnpart);

	cancelbtn = new QPushButton(btnpart);
	cancelbtn->setText(i18n("&Cancel"));
	cancelbtn->setFixedWidth(120);
	cancelbtn->setIconSet(QIconSet(loadIcon("stop.png")));
	QToolTip::add(cancelbtn, i18n("Cancel waiting for delivery"));
	cancelbtn->hide();

	sendbtn = new QPushButton(btnpart);
	sendbtn->setText(i18n("&Send"));
	sendbtn->setFixedWidth(120);
	sendbtn->setIconSet(QIconSet(loadIcon("forward.png")));
	connect(sendbtn, SIGNAL(clicked()), this, SLOT(sendMessage()));
	QAccel *acc = new QAccel(this);
	acc->connectItem(acc->insertItem(Key_Return + CTRL), this, SLOT(sendMessage()));

	acc = new QAccel(this);
	acc->connectItem(acc->insertItem(Key_PageUp + SHIFT), this, SLOT(pageUp()));
	acc = new QAccel(this);
	acc->connectItem(acc->insertItem(Key_PageDown + SHIFT), this, SLOT(pageDown()));

	btnpart->setStretchFactor(fillerbox, 50);
	btnpart->setStretchFactor(cancelbtn, 1);
	btnpart->setStretchFactor(sendbtn, 1);

	sizes.clear();
	sizes.append(50);
	sizes.append(1);
	split1->setSizes(sizes);

	QGridLayout *grid = new QGridLayout (this, 5, 4, 3, 3);
	grid->addMultiCellWidget(split1, 0, 4, 0, 3);
//	QHBoxLayout *subgrid = new QHBoxLayout();
//	subgrid->addWidget(edt, 50);
//	subgrid->addWidget(buttontray, 1);
//	if (userbox)
//		grid->addMultiCellWidget(split2, 0, 0, 0, 3);
//	else
//		grid->addMultiCellWidget(body, 0, 0, 0, 3);
//	grid->addMultiCellLayout(subgrid, 2, 2, 0, 3);
//	grid->addMultiCellWidget(edit, 3, 3, 0, 3);
//	grid->addWidget(cancelbtn, 4, 2);
//	grid->addWidget(sendbtn, 4, 3);
	grid->addRowSpacing(1, 5);
	grid->setRowStretch(0, 2);

	QMimeSourceFactory *bodyformat;
	bodyformat = new QMimeSourceFactory;

	body->setMimeSourceFactory(bodyformat);
	body->setTextFormat(Qt::RichText);

	connect(autosend, SIGNAL(clicked()), this, SLOT(regAutosend()));
	connect(history, SIGNAL(clicked()), this, SLOT(HistoryBox()));
	connect(iconsel, SIGNAL(clicked()), this, SLOT(insertEmoticon()));
	connect(whois, SIGNAL(clicked()), this, SLOT(userWhois()));
	connect(clearchat, SIGNAL(clicked()), this, SLOT(clearChatWindow()));
	connect(cancelbtn, SIGNAL(clicked()), this, SLOT(cancelMessage()));

	totaloccurences = 0;
	
	edit->setFocus();
}

Chat::~Chat() {
	int i,j;

	i = 0;
	while (i < chats.count() && chats[i].ptr != this)
		i++;
	chats.remove(chats.at(i));

	i = 0;
	while (i < acks.size() && acks[i].ptr != this)
		i++;
	if (i < acks.size()) {
		for (j = i + 1; j < acks.size(); j++) {
			acks[j-1].ack = acks[j].ack;
			acks[j-1].seq = acks[j].seq;
			acks[j-1].ptr = acks[j].ptr;
			acks[j-1].type = acks[j].type;
			}
		acks.resize(acks.size() - 1);
		}
	if (userbox)
		delete userbox;
		
	fprintf(stderr, "KK Chat::~Chat: chat destroyed: index %d\n", index);
}

void Chat::setupEncryptButton(bool enabled) {
#ifdef HAVE_OPENSSL
	encrypt_enabled = enabled;
	QToolTip::remove(encryption);
	if (enabled) {
		QToolTip::add(encryption, i18n("Disable encryption for this conversation"));
		encryption->setPixmap(loadIcon("encrypted.png"));
		}
	else {
		QToolTip::add(encryption, i18n("Enable encryption for this conversation"));
		encryption->setPixmap(loadIcon("decrypted.png"));
		}
#endif		
}

void Chat::pageUp() {
	body->scrollBy(0, (body->height() * -2) / 3);
}

void Chat::pageDown() {
	body->scrollBy(0, (body->height() * 2) / 3);
}

void Chat::setEncryptionBtnEnabled(bool enabled) {
#ifdef HAVE_OPENSSL
	encryption->setEnabled(enabled && config.encryption);
#endif
}

void Chat::changeAppearance() {
	if (uins.count() > 1 && userbox) {
		userbox->setPaletteBackgroundColor(config.colors.userboxBg);
		userbox->setPaletteForegroundColor(config.colors.userboxFg);
		userbox->QListBox::setFont(config.fonts.userbox);
		}
	body->setFont(config.fonts.chat);
	edit->setFont(config.fonts.chat);
}

void Chat::setTitle() {
	QString name;
	QString title;
	int j,k;
	UserListElement user;

	title = i18n("Chat with ");

	for (k = 0; k < uins.size(); k++) {
		if (k)
			title.append(", ");
    
		if (userlist.containsUin(uins[k])) {
			user = userlist.byUin(uins[k]);
			title.append(user.altnick);
			j = statusGGToStatusNr(user.status);
			if (!k)
				setIcon(QPixmap((const char**)gg_xpm[statusGGToStatusNr(user.status)]));
			else
				setIcon(QPixmap((const char**)gg_act_xpm));
			}
		else {
			title.append(QString::number(uins[k]));
			j = 0;
			setIcon(QPixmap((const char **)gg_inact_xpm));
			}
		title.append(" (");
		if (j == 1 || j == 3 || j == 5 || j == 7)
			title.append(i18n(statustext[j-1]));
		else
			title.append(i18n(statustext[j]));
		if (j & 1)
			title.append(i18n(": %1)").arg(user.description));
		else
			title.append(")");
		}
	title.replace(QRegExp("\n"), " ");

	setCaption(title);
	title_buffer = title;
}

void Chat::changeTitle() {
	if(!isActiveWindow()){
		if (caption() == "  "){
			setCaption(title_buffer);
			title_timer->start(500,TRUE);
		}
		else{    
			setCaption("  ");
			title_timer->start(500,TRUE);
		}
	}
}

void Chat::windowActivationChange(bool oldActive) {
	if (isActiveWindow() && title_timer->isActive()){
		title_timer->stop();
		setCaption(title_buffer);
	}
}

void Chat::keyPressEvent(QKeyEvent *e) {
	if (e->key() == Key_F9)
		clearChatWindow();
	else
		if (e->key() == Key_Escape)
			close();
	QWidget::keyPressEvent(e);
}

void Chat::regEncryptSend(void) {
#ifdef HAVE_OPENSSL
	encrypt_enabled = !encrypt_enabled;
	setupEncryptButton(encrypt_enabled);
#endif
}

/* register/unregister sending with Return key */
void Chat::regAutosend(void) {
	autosend_enabled = !autosend_enabled;
}

CustomInput::CustomInput(QWidget *parent, Chat *owner, const char *name) : QMultiLineEdit(parent, name) {
	tata = owner;
}

void CustomInput::keyPressEvent(QKeyEvent * e) {
	if (tata->autosend_enabled && ((e->key() == Key_Return) || (e->key() == Key_Enter))
		&& !(e->state() & ShiftButton))
		tata->sendMessage();
	else
		QMultiLineEdit::keyPressEvent(e);
}

/* convert special characters into emoticons, HTML into plain text and so forth */
QString Chat::convertCharacters(QString edit) {

	// escape'ujemy http:// i ftp:// zeby emotikony nie bruzdzily
	edit.replace(QRegExp("http://"),"___escaped_http___");
	edit.replace(QRegExp("ftp://"),"___escaped_ftp___");
	
	if(config.emoticons)
	{
		body->mimeSourceFactory()->addFilePath(emoticons.themePath());
		emoticons.expandEmoticons(edit);
	};
	
	// przywracamy http:// i ftp://
	edit.replace(QRegExp("___escaped_http___"),"http://");
	edit.replace(QRegExp("___escaped_ftp___"),"ftp://");
	
	// zmieniamy windowsowe \r\n na unixowe \n
	edit.replace( QRegExp("\r\n"), "\n" );
	edit.replace( QRegExp("<"), "&lt;" );
	edit.replace( QRegExp(">"), "&gt;" );
	edit.replace( QRegExp("__escaped_lt__"), "<");
	edit.replace( QRegExp("__escaped_gt__"), ">");
	edit.replace( QRegExp("  "), " &nbsp;" );
	edit.replace( QRegExp("\n"), "<BR>" );

	// detekcja adresow url
	QRegExp url_regexp("(http://|www\\.|ftp://)[a-zA-Z0-9\\-\\._/]+");
	for (int s=0; s<edit.length(); s++)
	{
		int p=url_regexp.search(edit,s);
		if (p < 0)
			break;
		int l=url_regexp.matchedLength();
		QString link="<a href=\""+edit.mid(p,l)+"\">"+
			edit.mid(p,l)+"</a>";
		edit=edit.left(p)+link+edit.mid(p+l);
		s+=link.length();
	};

	return edit;
}

/* unregister us */
void Chat::closeEvent(QCloseEvent *e) {
	fprintf(stderr, "KK Chat::closeEvent()\n");	
	QWidget::closeEvent(e);
}

/* look up party's info */
void Chat::userWhois(void) {
	SearchDialog *sd;
	QString tmp;
	uin_t uin;
	
	if (!userbox)
		uin = uins[0];
	else
		if (userbox->currentItem() == -1)
			uin = uins[0];
		else
			uin = userlist.byAltNick(userbox->currentText()).uin;
	sd = new SearchDialog(0, "User info", uin);
	sd->init();
	sd->show();
	sd->firstSearch();
}

void Chat::hyperlinkClicked(const QString &link) {
	QProcess *browser;
	QString cmd;
	QStringList args;

	if (config.defaultwebbrowser)
		cmd = QString("konqueror %1").arg(link);
	else {
                if (config.webbrowser == "") {
			QMessageBox::warning(this, i18n("WWW error"),
				i18n("Web browser was not specified. Visit the configuration section"));
			fprintf(stderr,"KK Chat::hyperlinkClicked(): Web browser NOT specified.\n");
			return;
			}
		cmd = QString(config.webbrowser).arg(link);
		}
	args = QStringList::split(" ", cmd);
	for (QStringList::iterator i = args.begin(); i != args.end(); i++)
		fprintf(stderr, "KK Chat::hyperlinkClicked(): %s\n", (*i).latin1());
	browser = new QProcess(this);
	browser->setArguments(args);
	if (!browser->start())
		QMessageBox::critical(this, i18n("WWW error"),
			i18n("Could not spawn Web browser process. Check if the Web browser is functional"));
//	QObject::connect(smsProcess, SIGNAL(processExited()), this, SLOT(smsSigHandler()));
	delete browser;
}

void Chat::formatMessage(bool me, QString &altnick, QString &msg, const char *time, QString &toadd) {
	QString editext = convertCharacters(msg);

	toadd.append("<TABLE width=\"100%\"><TR><TD bgcolor=\"");
	if (me)
	    	toadd.append(config.colors.mychatBg.name());
	else
	    	toadd.append(config.colors.usrchatBg.name());
	toadd.append("\">");
	toadd.append("<FONT color=\"");
	if (me)
		toadd.append(config.colors.mychatText.name());
	else
		toadd.append(config.colors.usrchatText.name());
	toadd.append("\"><B>");
	toadd.append(altnick);
	toadd.append(" ");
	toadd.append(__c2q(time));
	toadd.append("</B><BR>");
	toadd.append(editext);
	toadd.append("</TD></TR></TABLE></FONT>");
}

void Chat::scrollMessages(QString &toadd) {
	if (config.chatprune)
		pruneWindow();

	body->viewport()->setUpdatesEnabled(false);
	if (!config.scrolldown)
		body->setText(toadd + body->text());
	else {
		body->setText(body->text() + toadd);
		if (!lockscroll->isOn())
			body->scrollToBottom();
		}
	body->viewport()->setUpdatesEnabled(true);
	body->viewport()->repaint();
}

/* invoked from outside when new message arrives, this is the window to the world */
void Chat::checkPresence(UinsList senders, QString &msg, time_t time, QString &toadd) {
	formatMessage(false, userlist.byUin(senders[0]).altnick, msg, timestamp(time), toadd);

	scrollMessages(toadd);
}

void Chat::alertNewMessage(void) {
	if (config.playsoundchatinvisible){
		if (!isActiveWindow())
			playSound(config.soundchat);
		}
	else
		if (config.playsoundchat)
			playSound(config.soundchat);

	if (config.blinkchattitle)
		if (!isActiveWindow() && !title_timer->isActive())
			changeTitle();
}

void Chat::writeMyMessage() {
	QString toadd;

	formatMessage(true, config.nick, myLastMessage, timestamp(), toadd);
	scrollMessages(toadd);
	if (!edit->isEnabled())
		cancelMessage();
	edit->clear();
}

void Chat::addMyMessageToHistory() {
	int uin;

	QCString tmp(myLastMessage.local8Bit());		
	unsigned char *utmp = (unsigned char *) tmp.data();

	uin = uins[0];
	if (config.logmessages)
		appendHistory(uins, uin, (unsigned char *)utmp, true);
}

void Chat::clearChatWindow(void) {
	body->clear();
	totaloccurences = 0;
}

void Chat::cancelMessage(void) {
	edit->setReadOnly(false);
	edit->setEnabled(true);
	edit->setFocus();
	sendbtn->setEnabled(true);
	cancelbtn->hide();
}

/* sends the message typed */
void Chat::sendMessage(void) {
	int i,j;
	uin_t *users;
	
	if (getActualStatus() == GG_STATUS_NOT_AVAIL) {
		QMessageBox::critical(this, i18n("Send message error"),
			i18n("Application encountered network error."));
		return;
		}

	if (!QString::compare(edit->text().local8Bit(),""))
		return;

	myLastMessage = edit->text();

	if (edit->length() >= 2000)
		return;

	if (config.msgacks) {
		edit->setReadOnly(true);	
		edit->setEnabled(false);
		sendbtn->setEnabled(false);
		cancelbtn->show();
		}

	addMyMessageToHistory();
	// zmieniamy unixowe \n na windowsowe \r\n
	myLastMessage.replace(QRegExp("\n"), "\r\n");
			
	QCString tmp(myLastMessage.local8Bit());
	unsigned char *utmp = (unsigned char *) tmp.data();

	iso_to_cp(utmp);
	
	users = new (uin_t)[uins.count()];
	if (config.msgacks) {
		acks.resize(acks.size() + 1);
		i = acks.size() - 1;
		if (uins.count() > 1) {
			for (j = 0; j < uins.count(); j++)
				users[j] = uins[j];
			acks[i].seq = gg_send_message_confer(sess, GG_CLASS_CHAT,
				uins.count(), users, (unsigned char *)utmp);    
			acks[i].ack = uins.count();
			}
		else {
#ifdef HAVE_OPENSSL
			if (encrypt_enabled) {
				char* encrypted = sim_message_encrypt((unsigned char *)utmp, uins[0]);
				if (encrypted != NULL) {
					acks[i].seq = gg_send_message(sess, GG_CLASS_CHAT, uins[0], (unsigned char *)encrypted);
					acks[i].ack = 1;
					free(encrypted);
				}
			} else {
#endif
				acks[i].seq = gg_send_message(sess, GG_CLASS_CHAT, uins[0], (unsigned char *)utmp);
				acks[i].ack = 1;
#ifdef HAVE_OPENSSL
			}
#endif
		}

		acks[i].type = 2;
		acks[i].ptr = this;
		}
	else {
		if (uins.count() > 1) {
			for (j = 0; j < uins.count(); j++)
				users[j] = uins[j];
			gg_send_message_confer(sess, GG_CLASS_CHAT,
				uins.count(), users, (unsigned char *)utmp);    
			}
		else {
#ifdef HAVE_OPENSSL
			if (encrypt_enabled) {
				char* encrypted = sim_message_encrypt((unsigned char *)utmp, uins[0]);
				if (encrypted != NULL) {
					gg_send_message(sess, GG_CLASS_CHAT, uins[0], (unsigned char *)encrypted);
					free(encrypted);
				}
			} else {
#endif
				gg_send_message(sess, GG_CLASS_CHAT, uins[0], (unsigned char *)utmp);
#ifdef HAVE_OPENSSL
			}
#endif
		}
		writeMyMessage();	
	}
	delete users;

	if (sess->check & GG_CHECK_WRITE)
		kadusnw->setEnabled(true);		
}

/* prunes messages */
void Chat::pruneWindow(void) {
	int index,occurences;

	occurences = 0;
	if (config.scrolldown) {
		index = -1;
		while (occurences != config.chatprunelen && totaloccurences > config.chatprunelen - 1) {
			index = body->text().findRev(QString("<TABLE"), index - 8);
			occurences++;
			}
		totaloccurences++;

		body->setText(body->text().right(body->text().length() - index));	
		}
	else {
		index = 0;
		while (occurences != config.chatprunelen && totaloccurences > config.chatprunelen ) {
			if (occurences == 0)
				index = body->text().find(QString("<TABLE"), 0);
			else
				index = body->text().find(QString("<TABLE"), index + 8);

			occurences++;
			}
		totaloccurences++;

		if (totaloccurences > config.chatprunelen && index != -1 && index != 0)
			body->setText(body->text().left(index));
		}
}

/* opens messages history */
void Chat::HistoryBox(void) {
	History *hb;

	hb = new History(uins);
	hb->show();
}

/* this nifty icon selector */
void Chat::insertEmoticon(void) {
	if (iconsel_ptr == NULL) {
//		iconsel->setOn(true);
		iconsel_ptr = new IconSelector(NULL, "Icon selector", this);
		QPoint pos=iconsel->mapToGlobal(QPoint(0,0));
		iconsel_ptr->move(pos.x() - iconsel_ptr->sizeHint().width(),
			pos.y() + iconsel->height()/2 - iconsel_ptr->sizeHint().height()/2 );
		iconsel_ptr->show();
		}
	else {
//		iconsel->setOn(false);
		iconsel_ptr->close();
		iconsel_ptr = NULL;
		}
}

/* adds an emoticon code to the edit window */
void Chat::addEmoticon(QString string) {
	if (string.length()) {
		edit->setText(edit->text() + string);
		edit->end();
		edit->setFocus();
		}
	iconsel_ptr = NULL;
//	iconsel->setOn(false);
}

IconSelectorButton::IconSelectorButton(QWidget* parent,const QString& emoticon_string)
	: QToolButton(parent)
{
	EmoticonString = emoticon_string;
	connect(this, SIGNAL(clicked()), this, SLOT(buttonClicked()));
};

void IconSelectorButton::buttonClicked()
{
	emit clicked(EmoticonString);
};

/* the icon selector itself */
IconSelector::IconSelector(QWidget *parent, const char *name, Chat * caller) : QWidget (parent, name,Qt::WType_Popup) {
	callingwidget = caller;
	setWFlags(Qt::WDestructiveClose/*||Qt::WStyle_NoBorder||Qt::WStyle_NoBorderEx||Qt::WX11BypassWM*/);
	
	int emoticons_count=emoticons.emoticonsCount();
	int selector_width=(int)sqrt((double)emoticons_count);
	int btn_width=0;
	QGridLayout *grid = new QGridLayout(this, 0, selector_width, 0, 0);

	for(int i=0; i<emoticons_count; i++)
	{
		IconSelectorButton* btn = new IconSelectorButton(this,emoticons.emoticonString(i));
		btn->setPixmap(QPixmap(emoticons.emoticonPicPath(i)));
		btn->setAutoRaise(true);
		QToolTip::add(btn, emoticons.emoticonString(i));
		btn_width=btn->sizeHint().width();
		grid->addWidget(btn, i/selector_width, i%selector_width);
		connect(btn,SIGNAL(clicked(const QString&)),this,SLOT(iconClicked(const QString&)));
	};
};

void IconSelector::closeEvent(QCloseEvent *e) {
	callingwidget->addEmoticon("");
	QWidget::closeEvent(e);
}

void IconSelector::iconClicked(const QString& emoticon_string)
{
	callingwidget->addEmoticon(emoticon_string);
	close();
};

