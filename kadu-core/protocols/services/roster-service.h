/*
 * %kadu copyright begin%
 * Copyright 2011 Piotr Galiszewski (piotr.galiszewski@kadu.im)
 * Copyright 2009 Wojciech Treter (juzefwt@gmail.com)
 * Copyright 2009 Bartłomiej Zimoń (uzi18@o2.pl)
 * Copyright 2004 Adrian Smarzewski (adrian@kadu.net)
 * Copyright 2007, 2008, 2009, 2010, 2011 Rafał Malinowski (rafal.przemyslaw.malinowski@gmail.com)
 * Copyright 2011 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2004, 2006 Marcin Ślusarz (joi@kadu.net)
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

#ifndef ROSTER_SERVICE_H
#define ROSTER_SERVICE_H

#include <QtCore/QObject>
#include <QtCore/QQueue>

#include "buddies/buddy.h"
#include "contacts/contact.h"
#include "protocols/services/roster/roster-task.h"

#include "exports.h"

#include "protocols/services/protocol-service.h"

/**
 * @addtogroup Protocol
 * @{
 */

/**
 * @class RosterService
 * @author Rafał 'Vogel' Malinowski
 * @short Roster protocol service allows adding, removing and updating contacts on remote roster.
 *
 * This service allows adding, removing and updating contacts on remote roster. Every added contact is watched
 * for changes and updated automatically, until it is removed.
 *
 * If an action cannot be executed immediately it is stored as @link RosterTask @endlink object for later execution.
 * List of tasks is available by calling tasks() getter and can be changed by setTasks() setted. This allows for storing
 * and restoring this list between program invocations. Only one @link RosterTask @endlink for each contact id can be on
 * the list at a time. This service is responsible of choosing which task should be left on a list and which one should
 * be removed in case when second one is added for given id.
 *
 * When receiving updates from remote roster list of tasks is checked for id of contact from remote roster. Request for
 * contact removal or update from remote roster will be ignored if an update or add task is on the task list. This service
 * assumes that its changes are more important that these of remote roster. Update request are ignored if Detached flag
 * of @link RosterEntry @endlink of given contact is set to true.
 *
 * At begining of roster initialization all contacts of service's account that do not have any task are marked as deleted by
 * remote roster. During initialization deletion mark is removed from contacts that have data on remote roster. After
 * initialization rest of contacts is removed from local roster. In this case Detached flag of @link RosterEntry @endlink
 * does not count as it is only used for detaching from data synchronization.
 *
 * Signal rosterRead() is emited after calling prepareRoster() when implementation decides that initialization was finished.
 */
class KADUAPI RosterService : public ProtocolService
{
	Q_OBJECT

public:
	/**
	 * @enum RosterState
	 * @author Rafał 'Vogel' Malinowski
	 * @short State of roster service.
	 */
	enum RosterState {
		/**
		 * Roster service was not initialized and cannot perform any operation except prepareRoster().
		 */
		StateNonInitialized,
		/**
		 * Roster service is during prepareRoster() operation.
		 */
		StateInitializing,
		/**
		 * Roster is initialized and ready to accept local or remote changes.
		 */
		StateInitialized,
		/**
		 * Remote update was detected and is not being processed to update local roster.
		 */
		StateProcessingRemoteUpdate,
		/**
		 * Local update was detected and is not being processed to update remote roster.
		 */
		StateProcessingLocalUpdate
	};

private:
	RosterState State;
	QVector<Contact> Contacts;
	QQueue<RosterTask> Tasks;
	QMap<QString, RosterTask> IdToTask;

	/**
	 * @author Rafał 'Vogel' Malinowski
	 * @short Return true if task of second type should replace task of first type on list of task to execute.
	 * @return true if task of second type should replace task of first type on list of task to execute
	 */
	bool shouldReplaceTask(RosterTaskType taskType, RosterTaskType replacementType);

private slots:
	/**
	 * @author Rafał 'Vogel' Malinowski
	 * @short Slot called when protocol disconencted.
	 *
	 * Roster state is reset to StateNonInitialized.
	 */
	void disconnected();

	/**
	 * @enum RosterState
	 * @author Rafał 'Vogel' Malinowski
	 * @short Slot called when data of contact or contact's owner buddy changed.
	 *
	 * This slot can only by called for contacts that were previously added to roster using addContact() methods
	 * and were not removed.
	 */
	void contactUpdated();

protected:
	/**
	 * @author Rafał 'Vogel' Malinowski
	 * @short Return true if local update can be processed.
	 * @return true if local update can be processed
	 *
	 * Local update can only be processed when roster is in StateInitialized. Derivered services can override this
	 * method and add another conditions.
	 */
	virtual bool canPerformLocalUpdate() const;

	/**
	 * @author Rafał 'Vogel' Malinowski
	 * @short Sets state of roster service.
	 * @param state new state
	 */
	void setState(RosterState state);

	/**
	 * @author Rafał 'Vogel' Malinowski
	 * @short Add new task for later execution.
	 * @param task new task
	 *
	 * If existing task for given id is available then this service decided which one to use and which one to ignore.
	 * If exsiting task is delete then it is always replaced. Add task can be only replaced by delete task. Update task
	 * can be replaced by any non-update task.
	 */
	void addTask(const RosterTask &task);

	/**
	 * @author Rafał 'Vogel' Malinowski
	 * @short Execute given roster task.
	 * @param task to execut
	 *
	 * Default implementation of this method does nothing.
	 */
	virtual void executeTask(const RosterTask &task);

	/**
	 * @author Rafał 'Vogel' Malinowski
	 * @short Execute all stored RosterTask.
	 *
	 * This method executes all stored tasks. List of not-executed tasks will be empty after this call.
	 */
	void executeAllTasks();

protected slots:
	/**
	 * @enum RosterState
	 * @author Rafał 'Vogel' Malinowski
	 * @short Method called when contact's dirtness is changed.
	 *
	 * This method is only called for contacts that were previously added to roster using addContact() methods
	 * and were not removed and state of roster service is StateInitialized.
	 *
	 * Derivered services must reimplement this method.
	 */
	virtual void updateContact(const Contact &contact) = 0;

public:
	/**
	 * @author Rafał 'Vogel' Malinowski
	 * @short Create new instance of RosterService bound to given Protocol.
	 * @param protocol protocol to bound this service to
	 */
	explicit RosterService(Protocol *protocol);
	virtual ~RosterService();

	/**
	 * @author Rafał 'Vogel' Malinowski
	 * @short Return current state of this service.
	 * @return current state of this service
	 */
	RosterState state() const { return State; }

	/**
	 * @author Rafał 'Vogel' Malinowski
	 * @short Prepare roster to its work.
	 *
	 * This method must be reimplemented by derivered services. Depending on protocol it should download remote roster,
	 * upload local one, merge both or do nothig. After successfull (or not) preparation rosterReady() signal must be
	 * emited.
	 */
	virtual void prepareRoster() = 0;

	/**
	 * @author Rafał 'Vogel' Malinowski
	 * @short Return list of current non-executed roster tasks.
	 * @return list of current non-executed roster tasks
	 */
	QVector<RosterTask> tasks();

	/**
	 * @author Rafał 'Vogel' Malinowski
	 * @short Set list of non-executed roster tasks.
	 * @param tasks new list of non-executed roster tasks
	 */
	void setTasks(const QVector<RosterTask> tasks);

public slots:
	/**
	 * @author Rafał 'Vogel' Malinowski
	 * @short Add new contact to roster.
	 * @param contact new contact
	 *
	 * This method add new contact to roster. Derivered services should reimplement this method and call
	 * RosterService::addContact at begining and check it return value - when false, no remote adding should be done.
	 *
	 * This implementation adds contact to internal list. It also starts watching on changes on this contact.
	 */
	virtual void addContact(const Contact &contact);

	/**
	 * @author Rafał 'Vogel' Malinowski
	 * @short Remove contact from roster.
	 * @param contact contact to remove
	 *
	 * This method removes contact from roster. Derivered services should reimplement this method and call
	 * RosterService::removeContact at begining and check it return value - when false, no remote removing should be done.
	 *
	 * This implementation removes contact from internal list. It also stops watching on changes on this contact.
	 */
	virtual void removeContact(const Contact &contact);

signals:
	/**
	 * @author Rafał 'Vogel' Malinowski
	 * @short Signal emited when prepareRoster() operation is finished
	 * @param ok true, if preparing roster was successfull
	 */
	void rosterReady(bool ok);

};

/**
 * @}
 */

#endif // ROSTER_SERVICE_H
