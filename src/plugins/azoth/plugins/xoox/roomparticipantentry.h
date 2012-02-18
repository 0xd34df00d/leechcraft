/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_ROOMPARTICIPANTENTRY_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_ROOMPARTICIPANTENTRY_H
#include <memory>
#include <QObject>
#include <QStringList>
#include <QXmppMucIq.h>
#include "entrybase.h"

namespace LeechCraft
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
	{
		Q_OBJECT

		QString Nick_;
		RoomHandler *RoomHandler_;
		const QString ID_;

		QXmppMucItem::Affiliation Affiliation_;
		QXmppMucItem::Role Role_;
	public:
		RoomParticipantEntry (const QString&, RoomHandler*, GlooxAccount*);

		QObject* GetParentAccount () const ;
		QObject* GetParentCLEntry () const;
		Features GetEntryFeatures () const;
		EntryType GetEntryType () const;
		QString GetEntryName () const;
		void SetEntryName (const QString&);
		QString GetEntryID () const;
		QString GetHumanReadableID () const;
		QStringList Groups () const;
		void SetGroups (const QStringList&);
		QStringList Variants () const;
		QObject* CreateMessage (IMessage::MessageType,
				const QString&, const QString&);

		QString GetJID () const;
		QString GetRealJID () const;
		QString GetNick () const;

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
