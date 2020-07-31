/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QString>
#include <QXmppMucIq.h>
#include <QXmppDiscoveryIq.h>
#include <interfaces/azoth/azothcommon.h>

class QDomElement;
class QWidget;
class QDataStream;

class QXmppMessage;
class QXmppDataForm;
class QXmppPresence;

class GlooxAccount;

QDataStream& operator<< (QDataStream&, const QXmppDiscoveryIq::Identity&);
QDataStream& operator>> (QDataStream&, QXmppDiscoveryIq::Identity&);

namespace LC
{
namespace Azoth
{
struct EntryStatus;

namespace Xoox
{
class EntryBase;
class CapsDatabase;

namespace XooxUtil
{
	extern const QString NsRegister;

	QString RoleToString (const QXmppMucItem::Role&);
	QString AffiliationToString (const QXmppMucItem::Affiliation&);

	struct StaticClientInfo
	{
		QString ID_;
		QString HumanReadableName_;

		bool IsEmpty () const
		{
			return ID_.isEmpty () || HumanReadableName_.isEmpty ();
		}
	};
	StaticClientInfo GetStaticClientInfo (const QString&);

	QDomElement XmppElem2DomElem (const QXmppElement&);
	QXmppElement Form2XmppElem (const QXmppDataForm&);

	bool RunFormDialog (QWidget*);

	bool CheckUserFeature (EntryBase *entry,
			const QString& variant, const QString& feature, const CapsDatabase *capsDB);

	QXmppMessage Forwarded2Message (const QXmppElement& wrapper);

	EntryStatus PresenceToStatus (const QXmppPresence& pres);

	QXmppPresence StatusToPresence (State, const QString&, int);

	QString GetBareJID (const QString& entryId, GlooxAccount *acc);
}
}
}
}
