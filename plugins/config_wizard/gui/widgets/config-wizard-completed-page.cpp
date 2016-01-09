/*
 * %kadu copyright begin%
 * Copyright 2013, 2014 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2011, 2013, 2014 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include <QtWidgets/QCheckBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QLabel>

#include "accounts/account-manager.h"
#include "core/core.h"
#include "gui/windows/your-accounts.h"

#include "config-wizard-completed-page.h"

ConfigWizardCompletedPage::ConfigWizardCompletedPage(QWidget *parent) :
		ConfigWizardPage(parent)
{
	setDescription(tr("<p>Confirm your wizard setting to start using Kadu.</p>"));

	createGui();
}

ConfigWizardCompletedPage::~ConfigWizardCompletedPage()
{
}

void ConfigWizardCompletedPage::createGui()
{
	formLayout()->addRow(new QLabel(tr("<h3>Configuration Wizard Completed</h3>"), this));

	ConfigureAccouuntsCheckBox = new QCheckBox(tr("Go to Accounts Setting after closing this window"), this);

	formLayout()->addRow(QString(), ConfigureAccouuntsCheckBox);
}

void ConfigWizardCompletedPage::initializePage()
{
	ConfigureAccouuntsCheckBox->setChecked(Core::instance()->accountManager()->items().isEmpty());
}

void ConfigWizardCompletedPage::acceptPage()
{
    if (ConfigureAccouuntsCheckBox->isChecked())
		YourAccounts::instance()->show();
}

#include "moc_config-wizard-completed-page.cpp"
