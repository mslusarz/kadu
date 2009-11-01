 /***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CONTACTS_MODEL_PROXY
#define CONTACTS_MODEL_PROXY

#include <QtGui/QSortFilterProxyModel>

#include "abstract-contacts-model.h"
#include "buddies/filter/abstract-buddy-filter.h"

class ContactsModelProxy : public QSortFilterProxyModel, public AbstractContactsModel
{
	Q_OBJECT

	AbstractContactsModel *SourceContactModel;
	QList<AbstractBuddyFilter *> Filters;

	bool BrokenStringCompare;
	int compareNames(QString n1, QString n2) const;
	
private slots:
	void modelDestroyed();;

protected:
	virtual bool lessThan(const QModelIndex &left, const QModelIndex &right) const;
	virtual bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;

public:
	ContactsModelProxy(QObject *parent = 0);

	virtual void setSourceModel(QAbstractItemModel *sourceModel);
	void addFilter(AbstractBuddyFilter *filter);
	void removeFilter(AbstractBuddyFilter *filter);

	// AbstractContactsModel implementation
	virtual Buddy contact(const QModelIndex &index) const;
	virtual const QModelIndex contactIndex(Buddy contact) const;

};

#endif // CONTACTS_MODEL_PROXY
