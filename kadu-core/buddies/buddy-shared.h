/*
 * %kadu copyright begin%
 * Copyright 2010, 2011, 2012 Piotr Galiszewski (piotr.galiszewski@kadu.im)
 * Copyright 2010 Piotr Dąbrowski (ultr@ultr.pl)
 * Copyright 2009 Bartłomiej Zimoń (uzi18@o2.pl)
 * Copyright 2010, 2011, 2013, 2014 Bartosz Brachaczek (b.brachaczek@gmail.com)
 * Copyright 2009, 2010, 2011, 2012, 2013, 2014 Rafał Przemysław Malinowski (rafal.przemyslaw.malinowski@gmail.com)
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

#include "buddies/buddy-gender.h"
#include "exports.h"
#include "misc/change-notifier.h"
#include "storage/shared.h"

#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QPointer>
#include <QtCore/QSet>
#include <QtCore/QSharedData>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QUuid>
#include <QtXml/QDomElement>
#include <injeqt/injeqt.h>

#define BuddyShared_PropertyDirtyWrite(type, name, capitalized_name) \
    void set##capitalized_name(type name)                            \
    {                                                                \
        ensureLoaded();                                              \
        if (capitalized_name != name)                                \
        {                                                            \
            capitalized_name = name;                                 \
            changeNotifier().notify();                               \
            markContactsDirty();                                     \
        }                                                            \
    }

#define BuddyShared_PropertyDirty(type, name, capitalized_name) \
    KaduShared_PropertyRead(type, name, capitalized_name) BuddyShared_PropertyDirtyWrite(type, name, capitalized_name)

#define BuddyShared_PropertySubscriptionRead(capitalized_name) \
    bool is##capitalized_name()                                \
    {                                                          \
        ensureLoaded();                                        \
        return capitalized_name;                               \
    }

#define BuddyShared_PropertySubscriptionWrite(capitalized_name) \
    void set##capitalized_name(bool name)                       \
    {                                                           \
        ensureLoaded();                                         \
        if (capitalized_name != name)                           \
        {                                                       \
            capitalized_name = name;                            \
            buddySubscriptionChanged();                         \
            changeNotifier().notify();                          \
        }                                                       \
    }

#define BuddyShared_PropertySubscriptionDirtyWrite(capitalized_name) \
    void set##capitalized_name(bool name)                            \
    {                                                                \
        ensureLoaded();                                              \
        if (capitalized_name != name)                                \
        {                                                            \
            capitalized_name = name;                                 \
            buddySubscriptionChanged();                              \
            changeNotifier().notify();                               \
            markContactsDirty();                                     \
        }                                                            \
    }

#define BuddyShared_PropertySubscription(capitalized_name) \
    BuddyShared_PropertySubscriptionRead(capitalized_name) BuddyShared_PropertySubscriptionWrite(capitalized_name)

#define BuddyShared_PropertySubscriptionDirty(capitalized_name) \
    BuddyShared_PropertySubscriptionRead(capitalized_name) BuddyShared_PropertySubscriptionDirtyWrite(capitalized_name)

class Account;
class Avatars;
class BuddyManager;
class ConfigurationApi;
class Contact;
class GroupManager;
class Group;
class Myself;

class KADUAPI BuddyShared : public Shared
{
    Q_OBJECT

    QPointer<Avatars> m_avatars;
    QPointer<BuddyManager> m_buddyManager;
    QPointer<GroupManager> m_groupManager;
    QPointer<Myself> m_myself;

    bool CollectingGarbage;

    QList<Contact> Contacts;

    QString Display;
    QString FirstName;
    QString LastName;
    QString FamilyName;
    QString City;
    QString FamilyCity;
    QString NickName;
    QString HomePhone;
    QString Mobile;
    QString Email;
    QString Website;
    unsigned short BirthYear;
    BuddyGender Gender;
    QSet<Group> Groups;
    bool PreferHigherStatuses;

    bool Anonymous;
    bool Temporary;
    bool Blocked;
    bool OfflineTo;

    bool doAddToGroup(const Group &group);
    bool doRemoveFromGroup(const Group &group);

    int priorityForNewContact();

    void import_4_0_avatar();

private slots:
    INJEQT_SET void setAvatars(Avatars *avatars);
    INJEQT_SET void setBuddyManager(BuddyManager *buddyManager);
    INJEQT_SET void setGroupManager(GroupManager *groupManager);
    INJEQT_SET void setMyself(Myself *myself);
    INJEQT_INIT void init();

    void groupAboutToBeRemoved();
    void markContactsDirty();

protected:
    virtual std::shared_ptr<StoragePoint> createStoragePoint();

    virtual void load();
    virtual void store();
    virtual bool shouldStore();

public:
    explicit BuddyShared(const QUuid &uuid = QUuid());
    virtual ~BuddyShared();

    void collectGarbage();

    virtual StorableObject *storageParent();
    virtual QString storageNodeName();

    void importConfiguration(const QDomElement &parent);
    void importConfiguration();   // imports configuration from custom data values

    virtual void aboutToBeRemoved();

    QString id(const Account &account);

    void addContact(const Contact &contact);
    void removeContact(const Contact &contact);
    QVector<Contact> contacts(const Account &account);
    const QList<Contact> &contacts();

    void sortContacts();
    void normalizePriorities();

    bool isEmpty(bool checkOnlyForContacts);

    KaduShared_PropertyRead(const QSet<Group> &, groups, Groups) void setGroups(const QSet<Group> &groups);
    bool showInAllGroup();
    bool isInGroup(const Group &group);
    void addToGroup(const Group &group);
    void removeFromGroup(const Group &group);

    KaduShared_PropertyRead(const QString &, display, Display) void setDisplay(const QString &display);

    BuddyShared_PropertyDirty(const QString &, firstName, FirstName) BuddyShared_PropertyDirty(
        const QString &, lastName, LastName) BuddyShared_PropertyDirty(const QString &, familyName, FamilyName)
        BuddyShared_PropertyDirty(const QString &, city, City)
            BuddyShared_PropertyDirty(const QString &, familyCity, FamilyCity)
                BuddyShared_PropertyDirty(const QString &, nickName, NickName)
                    BuddyShared_PropertyDirty(const QString &, homePhone, HomePhone)
                        BuddyShared_PropertyDirty(const QString &, mobile, Mobile)
                            BuddyShared_PropertyDirty(const QString &, email, Email)
                                BuddyShared_PropertyDirty(const QString &, website, Website)
                                    KaduShared_Property(unsigned short, birthYear, BirthYear)
                                        KaduShared_Property(BuddyGender, gender, Gender)
                                            KaduShared_Property(bool, preferHigherStatuses, PreferHigherStatuses)
                                                BuddyShared_PropertySubscription(Anonymous)
                                                    KaduShared_PropertyBool(Temporary)
                                                        BuddyShared_PropertySubscriptionDirty(Blocked)
                                                            BuddyShared_PropertySubscriptionDirty(OfflineTo)

                                                                quint16 unreadMessagesCount();

signals:
    void contactAboutToBeAdded(const Contact &contact);
    void contactAdded(const Contact &contact);
    void contactAboutToBeRemoved(const Contact &contact);

    /**
     * @note When this signal is emited contact.ownerBuddy value is still set to this buddy
     */
    void contactRemoved(const Contact &contact);

    void updated();
    void displayUpdated();
    void buddySubscriptionChanged();
};
