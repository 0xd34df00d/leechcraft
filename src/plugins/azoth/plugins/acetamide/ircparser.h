/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
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

#ifndef PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCPARSER_H
#define PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCPARSER_H

#include <boost/shared_ptr.hpp>
#include <QObject>
#include "core.h"
#include "localtypes.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{

	class IrcServerHandler;

	class IrcParser : public QObject
	{
		Q_OBJECT

		IrcServerHandler *ISH_;
		ServerOptions ServerOptions_;
		IrcMessageOptions IrcMessageOptions_;
	public:
		IrcParser (IrcServerHandler*);
		void AuthCommand ();
		void UserCommand ();
		void NickCommand (const QStringList&);
		void JoinCommand (const QString&);
		void PrivMsgCommand (const QString&, const QString&);
		void PartCommand (const QStringList&);
		void PongCommand (const QStringList&);
		void RawCommand (const QStringList&);
		void CTCPRequest (const QStringList&);
		void CTCPReply (const QStringList&);
		void TopicCommand (const QStringList&);
		void NamesCommand (const QStringList&);
		void InviteCommand (const QStringList&);
		void KickCommand (const QStringList&);
		void OperCommand (const QStringList&);
		void SQuitCommand (const QStringList&);
		void MOTDCommand (const QStringList&);
		void LusersCommand (const QStringList&);
		void VersionCommand (const QStringList&);
		void StatsCommand (const QStringList&);
		void LinksCommand (const QStringList&);
		void TimeCommand (const QStringList&);
		void ConnectCommand (const QStringList&);
		void TraceCommand (const QStringList&);
		void AdminCommand (const QStringList&);
		void InfoCommand (const QStringList&);
		void WhoCommand (const QStringList&);
		void WhoisCommand (const QStringList&);
		void WhowasCommand (const QStringList&);
		void KillCommand (const QStringList&);
		void PingCommand (const QStringList&);
		void AwayCommand (const QStringList&);
		void RehashCommand (const QStringList&);
		void DieCommand (const QStringList&);
		void RestartCommand (const QStringList&);
		void SummonCommand (const QStringList&);
		void UsersCommand (const QStringList&);
		void UserhostCommand (const QStringList&);
		void WallopsCommand (const QStringList&);
		void IsonCommand (const QStringList&);

		bool ParseMessage (const QString&);
		IrcMessageOptions GetIrcMessageOptions () const;
	};
};
};
};
#endif // PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCPARSER_H
