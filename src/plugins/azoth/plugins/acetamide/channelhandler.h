/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010  Oleg Linkin
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
#include <interfaces/imucentry.h>
#include "clientconnection.h"
#include "channelparticipantentry.h"
#include "core.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	class ChannelCLEntry;
	class IrcAccount;
	class IrcMessage;
	class ChannelParticipantEntry;
	
	class ChannelHandler : public QObject
	{
		Q_OBJECT
		
		IrcAccount *Account_;
		ChannelCLEntry *CLEntry_;
		QHash<QString, ChannelParticipantEntry_ptr> Nick2Entry_;
		QString ChannelID_;
		QString Nickname_;
		QString Subject_;
		ChannelOptions Channel_;
		ServerOptions Server_;
	public:
		ChannelHandler (const ServerOptions&, const ChannelOptions&, IrcAccount*);
		QString GetChannelID () const;
		ChannelCLEntry* GetCLEntry () const;
		QList<QObject*> GetParticipants () const;
		
		IrcMessage* CreateMessage (IMessage::MessageType,
				const QString&, const QString&);
		
		QString GetNickname () const;
		QString SetNickname (const QString&);
		QString GetSubject () const;
		void SetSubject (const QString&);
		void Leave (const QString&);
		void SetChannelUser (const QString&);
		void HandleMessage (const QString&, const QString&);
		ChannelOptions GetChannelOptions () const;
		ServerOptions GetServerOptions () const;
		ChannelParticipantEntry_ptr GetParticipantEntry (const QString&, bool announce = true);
	private:
		ChannelParticipantEntry_ptr CreateParticipantEntry (const QString&, bool announce);
		void RemoveThis ();
	};
};
};
};

#endif // PLUGINS_AZOTH_PLUGINS_ACETAMIDE_CHANNELHANDLER_H
