/*
 * %kadu copyright begin%
 * Copyright 2010, 2010, 2011 Piotr Galiszewski (piotr.galiszewski@kadu.im)
 * Copyright 2011 Piotr Dąbrowski (ultr@ultr.pl)
 * Copyright 2010, 2011, 2013 Rafał Malinowski (rafal.przemyslaw.malinowski@gmail.com)
 * Copyright 2010, 2011, 2013 Bartosz Brachaczek (b.brachaczek@gmail.com)
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

#include "sound-select-file.h"

#include "sound-manager.h"

#include "configuration/configuration.h"
#include "configuration/deprecated-configuration-api.h"
#include "core/application.h"
#include "gui/widgets/configuration/notify-group-box.h"
#include "gui/widgets/select-file.h"
#include "icons/kadu-icon.h"

#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QPushButton>

SoundSelectFile::SoundSelectFile(SoundManager *manager, QWidget *parent) :
		QWidget{parent},
		m_manager{manager}
{
	auto testButton = new QPushButton{KaduIcon{"external_modules/mediaplayer-media-playback-play"}.icon(), QString{}, this};
	connect(testButton, SIGNAL(clicked()), this, SLOT(test()));

	m_selectFile = new SelectFile{"audio", this};
	connect(m_selectFile, SIGNAL(fileChanged()), this, SIGNAL(fileChanged()));

	auto layout = new QHBoxLayout{this};
	layout->setSpacing(0);
	layout->setMargin(0);
	layout->addWidget(testButton);
	layout->addWidget(m_selectFile);
}

SoundSelectFile::~SoundSelectFile()
{
}

QString SoundSelectFile::file() const
{
	return m_selectFile->file();
}

void SoundSelectFile::setFile(const QString& file)
{
	m_selectFile->setFile(file);
}

void SoundSelectFile::test()
{
	m_manager->playFile(m_selectFile->file(), true);
}

#include "moc_sound-select-file.cpp"