/*
 * %kadu copyright begin%
 * Copyright 2010, 2011 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2009, 2010, 2011 Rafał Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#ifndef ACCOUNTS_COMBO_BOX_H
#define ACCOUNTS_COMBO_BOX_H

#include "accounts/account.h"
#include "gui/widgets/kadu-combo-box.h"

class AbstractAccountFilter;
class AccountsModel;
class AccountsProxyModel;

class AccountsComboBox : public KaduComboBox
{
	Q_OBJECT

	AccountsModel *Model;
	AccountsProxyModel *ProxyModel;

private slots:
	void currentIndexChangedSlot(int index);

protected:
	virtual bool compare(QVariant value, QVariant previousValue) const;

public:
	explicit AccountsComboBox(bool includeSelectAccount, QWidget *parent = 0);
	virtual ~AccountsComboBox();

	void setCurrentAccount(Account account);
	Account currentAccount();

	void setIncludeIdInDisplay(bool includeIdInDisplay);

	void addFilter(AbstractAccountFilter *filter);
	void removeFilter(AbstractAccountFilter *filter);

signals:
	void accountChanged(Account account, Account lastAccount = Account::null);

};

#endif // ACCOUNTS_COMBO_BOX_H
