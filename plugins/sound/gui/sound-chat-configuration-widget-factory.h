/*
 * %kadu copyright begin%
 * Copyright 2015 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#pragma once

#include "widgets/chat-configuration-widget-factory.h"

#include <QtCore/QPointer>
#include <injeqt/injeqt.h>

class PluginInjectedFactory;
class SoundManager;

class SoundChatConfigurationWidgetFactory : public QObject, public ChatConfigurationWidgetFactory
{
	Q_OBJECT

public:
	Q_INVOKABLE SoundChatConfigurationWidgetFactory(QObject *parent = nullptr);
	virtual ~SoundChatConfigurationWidgetFactory();

	virtual ChatConfigurationWidget * createWidget(const Chat &chat, QWidget *parent);

private:
	QPointer<PluginInjectedFactory> m_pluginInjectedFactory;
	QPointer<SoundManager> m_soundManager;

private slots:
	INJEQT_SET void setPluginInjectedFactory(PluginInjectedFactory *pluginInjectedFactory);
	INJEQT_SET void setSoundManager(SoundManager *soundManager);

};
