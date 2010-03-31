#ifndef EXPIMP_H
#define EXPIMP_H

#include <QtCore/QList>
#include <QtGui/QWidget>

#include "userlistelement.h"

class QLabel;
class QPushButton;
class QTreeWidget;

/**
	Dialog umo�liwiaj�cy import / export listy kontakt�w z serwera GG
	\class UserlistImportExport
	\brief Import / Export userlisty
**/
class UserlistImportExport : public QWidget {
	Q_OBJECT

	QPushButton *pb_fetch; /*!< przycisk pobierania listy */
	QList<UserListElement> importedUserlist;
	QPushButton *pb_send; /*!< przycisk wysy�ania listy */
	QPushButton *pb_delete; /*!< przycisk kasuj�cy list� */
	QPushButton *pb_tofile;
	QLabel *l_itemscount;
	QLabel *l_importcount;
	QTreeWidget *lv_userlist;

private slots:
	void startImportTransfer(void);
	void makeUserlist(void);
	void updateUserlist(void);
	void fromfile();
	void userListImported(bool ok, QList<UserListElement> userList);
	void startExportTransfer(void);
	void clean(void);
	void ExportToFile(void);
	void userListExported(bool ok);
	void userListCleared(bool ok);
	void keyPressEvent(QKeyEvent *);
	void updateUserListCount();

public:
	/**
		Konstruktor okna importu / exportu
		\fn UserlistImportExport(QWidget *parent = 0)
		\param parent rodzic okna
		\param name nazwa obiektu
	**/
	UserlistImportExport(QWidget *parent = 0);

	/**
		\fn ~UserlistImportExport()
		Destruktor okna importu / exportu
	**/
	~UserlistImportExport();

};

#endif
