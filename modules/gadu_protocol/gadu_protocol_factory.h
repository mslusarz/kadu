/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef GADU_PROTOCOL_FACTORY_H
#define GADU_PROTOCOL_FACTORY_H

#include "gadu_configuration_dialog.h"
#include "protocols/protocol_factory.h"

class GaduProtocolFactory : public ProtocolFactory
{
public:
	virtual Protocol * newInstance();
	virtual AccountData * newAccountData();
	virtual GaduConfigurationDialog * newConfigurationDialog(AccountData *, QWidget *);

	virtual QString name() { return "gadu"; }
	virtual QString displayName() { return tr("Gadu-Gadu"); }
	virtual QString iconName() { return "BigOnline"; }

};

#endif // GADU_PROTOCOL_FACTORY_H
