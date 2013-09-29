/*
 * %kadu copyright begin%
 * Copyright 2013 Rafał Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include <QtGui/QFormLayout>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>

#include "accounts/account.h"
#include "protocols/protocol.h"
#include "protocols/protocol-factory.h"

#include "gui/windows/otr-peer-identity-verification-window.h"
#include "otr-fingerprint-service.h"
#include "otr-peer-identity-verification-service.h"

#include "otr-peer-identity-verification-respond-question-and-answer-page.h"

OtrPeerIdentityVerificationRespondQuestionAndAnswerPage::OtrPeerIdentityVerificationRespondQuestionAndAnswerPage(const Contact &contact, QWidget *parent) :
		QWizardPage(parent), MyContact(contact)
{
	createGui();
}

OtrPeerIdentityVerificationRespondQuestionAndAnswerPage::~OtrPeerIdentityVerificationRespondQuestionAndAnswerPage()
{
}

void OtrPeerIdentityVerificationRespondQuestionAndAnswerPage::createGui()
{
	setCommitPage(true);
	setTitle(tr("Respond to Question and Answer"));

	QFormLayout *layout = new QFormLayout(this);

	QuestionLabel = new QLabel();
	QLineEdit *answerEdit = new QLineEdit();

	layout->addRow(new QLabel(tr("Question:")), QuestionLabel);
	layout->addRow(new QLabel(tr("Answer:")), answerEdit);

	registerField("respondQuestion", QuestionLabel, "text");
	registerField("respondAnswer", answerEdit);
}

void OtrPeerIdentityVerificationRespondQuestionAndAnswerPage::setPeerIdentityVerificationService(OtrPeerIdentityVerificationService *peerIdentityVerificationService)
{
	PeerIdentityVerificationService = peerIdentityVerificationService;
}

int OtrPeerIdentityVerificationRespondQuestionAndAnswerPage::nextId() const
{
	return OtrPeerIdentityVerificationWindow::ProgressPage;
}

void OtrPeerIdentityVerificationRespondQuestionAndAnswerPage::initializePage()
{
	setField("respondAnswer", QString());
}

bool OtrPeerIdentityVerificationRespondQuestionAndAnswerPage::validatePage()
{
	if (PeerIdentityVerificationService)
		PeerIdentityVerificationService.data()->respondVerification(MyContact, field("respondAnswer").toString());

	return true;
}