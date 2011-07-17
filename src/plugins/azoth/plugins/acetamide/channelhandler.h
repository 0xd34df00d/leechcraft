/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011 Oleg Linkin
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

#ifndef PLUGINS_AZOTH_PLUGINS_ACETAMIDE_CHANNELHANDLER_H
#define PLUGINS_AZOTH_PLUGINS_ACETAMIDE_CHANNELHANDLER_H

#include <QObject>
#include <QHash>
#include <interfaces/imessage.h>
#include "localtypes.h"
#include "serverparticipantentry.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	class ChannelCLEntry;
	class IrcMessage;
	class IrcServerHandler;

	class ChannelHandler : public QObject
	{
		Q_OBJECT
		QString ChannelID_;
		QString Subject_;
		ChannelCLEntry *ChannelCLEntry_;
		IrcServerHandler *ISH_;
		ChannelOptions ChannelOptions_;
		bool IsRosterReceived_;
		QHash<QString, ServerParticipantEntry_ptr> Nick2Entry_;
	public:
		ChannelHandler (IrcServerHandler*, const ChannelOptions&);
		QString GetChannelID () const;
		ChannelCLEntry* GetCLEntry () const;
		IrcServerHandler* GetIrcServerHandler () const;
		ChannelOptions GetChannelOptions () const;
		QList<QObject*> GetParticipants () const;

		ServerParticipantEntry_ptr GetSelf ();

		IrcMessage* CreateMessage (IMessage::MessageType,
				const QString&, const QString&);

		bool IsRosterReceived () const;
		void SetRosterReceived (bool);

		void ShowServiceMessage (const QString&, IMessage::MessageType,
				IMessage::MessageSubType);

		void SendPublicMessage (const QString&);
		void HandleIncomingMessage (const QString&, const QString&);
		void SetChannelUser (const QString&);
		// 0 - leave , 1 - kick, 2 - ban
		void RemoveChannelUser (const QString&, const QString&, int,
				const QString& who = QString ());

		void MakeJoinMessage (const QString&);
		void MakeLeaveMessage (const QString&, const QString&);
		void MakeKickMessage (const QString&, const QString&,
				const QString&);

		void SetMUCSubject (const QString&);
		QString GetMUCSubject () const;

		void LeaveChannel (const QString&, bool cmd = false);

		void RemoveThis ();
	};
};
};
};

#endif // PLUGINS_AZOTH_PLUGINS_ACETAMIDE_CHANNELHANDLER_H
