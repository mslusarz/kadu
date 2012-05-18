/*
 * %kadu copyright begin%
 * Copyright 2011 Piotr Galiszewski (piotr.galiszewski@kadu.im)
 * Copyright 2011 Piotr Dąbrowski (ultr@ultr.pl)
 * Copyright 2009, 2010, 2011 Rafał Malinowski (rafal.przemyslaw.malinowski@gmail.com)
 * Copyright 2010 Bartosz Brachaczek (b.brachaczek@gmail.com)
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

#include <QtGui/QApplication>

#include "network-manager-ntrack.h"

NetworkManagerNTrack::NetworkManagerNTrack()
{
	// fuck QtNtrack
	int argc = qApp->argc();
	char **argv = qApp->argv();
	QNtrack::instance()->init(&argc, &argv);

	connect(QNtrack::instance(), SIGNAL(stateChanged(QNTrackState,QNTrackState)), this, SLOT(stateChanged(QNTrackState,QNTrackState)));
}

NetworkManagerNTrack::~NetworkManagerNTrack()
{
}

bool NetworkManagerNTrack::isOnline(QNTrackState state) const
{
	return state == NTRACK_STATE_ONLINE || state == NTRACK_STATE_UNKNOWN;
}

bool NetworkManagerNTrack::isOnline()
{
	return isOnline(QNtrack::instance()->networkState());
}

void NetworkManagerNTrack::stateChanged(QNTrackState oldState, QNTrackState newState)
{
	bool wasOnline = isOnline(oldState);
	bool nowOnline = isOnline(newState);

	if (wasOnline != nowOnline)
		onlineStateChanged(nowOnline);
}
