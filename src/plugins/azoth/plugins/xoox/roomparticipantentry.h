/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_ROOMPARTICIPANTENTRY_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_ROOMPARTICIPANTENTRY_H
#include <memory>
#include <QObject>
#include <QStringList>
#include <QXmppMucIq.h>
#include "entrybase.h"

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	class GlooxAccount;
	class RoomPublicMessage;
	class GlooxMessage;
	class RoomHandler;

	class RoomParticipantEntry : public EntryBase
							   , public std::enable_shared_from_this<RoomParticipantEntry>
	{
		Q_OBJECT

		QString Nick_;
		RoomHandler *RoomHandler_;

		QXmppMucItem::Affiliation Affiliation_;
		QXmppMucItem::Role Role_;
	public:
		RoomParticipantEntry (const QString&, RoomHandler*, GlooxAccount*);

		ICLEntry* GetParentCLEntry () const;
		Features GetEntryFeatures () const;
		EntryType GetEntryType () const;
		QString GetEntryName () const;
		void SetEntryName (const QString&);
		QString GetEntryID () const;
		QStringList Groups () const;
		void SetGroups (const QStringList&);
		QStringList Variants () const;
		IMessage* CreateMessage (IMessage::Type,
				const QString&, const QString&);

		QString GetJID () const;
		QString GetRealJID () const;
		QString GetNick () const;

		void StealMessagesFrom (RoomParticipantEntry*);

		QXmppMucItem::Affiliation GetAffiliation () const;
		void SetAffiliation (QXmppMucItem::Affiliation);
		QXmppMucItem::Role GetRole () const;
		void SetRole (QXmppMucItem::Role);
	};

	typedef std::shared_ptr<RoomParticipantEntry> RoomParticipantEntry_ptr;
}
}
}

#endif
