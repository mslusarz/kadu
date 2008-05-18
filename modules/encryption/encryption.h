#ifndef ENCRYPTION_H
#define ENCRYPTION_H

#include <QDialog>

#include "main_configuration_window.h"

class ActionDescription;
class ChatWidget;
class KeysManager;
class Protocol;
class UserGroup;

/**
 * @defgroup encryption Encryption
 * @{
 */
class EncryptionManager : public ConfigurationUiHandler
{
	Q_OBJECT

		int MenuId;

		QMap<ChatWidget *, bool> EncryptionEnabled;
		QMap<ChatWidget *, bool> EncryptionPossible;

		void setupEncryptButton(ChatWidget* chat, bool enabled);
		void setupEncryptionButtonForUsers(UserListElements users, bool enabled);
		ActionDescription* action;

		QWidget *configurationWindow;
		void createDefaultConfiguration();

		KeysManager *KeysManagerDialog;

	private slots:
		void decryptMessage(Protocol *protocol, UserListElements senders, QString &msg, QByteArray &formats, bool &ignore);
		void sendMessageFilter(const UserListElements users, QString &msg, bool &stop);
		void setupEncrypt(const UserGroup *group);

		void generateMyKeys();
		void sendPublicKey();
		void encryptionActionActivated(QAction *sender, bool toggled);
		void userBoxMenuPopup();

		void keysManagerDialogDestroyed();

		void turnEncryption(UserGroup *group, bool on);
		void keyRemoved(UserListElement ule);
		void keyAdded(UserListElement ule);

	public:
		EncryptionManager();
		virtual ~EncryptionManager();

		virtual void mainConfigurationWindowCreated(MainConfigurationWindow *mainConfigurationWindow);

	public slots:
		void showKeysManagerDialog();

};

class SavePublicKey : public QDialog
{
	Q_OBJECT

		UserListElement user;
		QString keyData;

	private slots:
		void yesClicked();

	public:
		SavePublicKey(UserListElement user, QString keyData, QWidget *parent = 0);

	signals:
		void keyAdded(UserListElement ule);

};

/** @} */

#endif
