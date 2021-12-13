/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include "localtypes.h"

namespace LC::Azoth::Acetamide
{
	class IrcServerHandler;

	class IrcParser : public QObject
	{
		IrcServerHandler *ISH_;
		ServerOptions ServerOptions_;
		IrcMessageOptions IrcMessageOptions_;
	public:
		explicit IrcParser (IrcServerHandler*);

		void AuthCommand ();
		void UserCommand ();
		void QuoteCommand (const QStringList&);
		void NickCommand (const QStringList&);
		void JoinCommand (const QStringList&);
		void PrivMsgCommand (const QStringList&);
		void PartCommand (const QStringList&);
		void PongCommand (const QStringList&);
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
		void QuitCommand (const QStringList&);
		void ChanModeCommand (const QStringList&);
		void ChannelsListCommand (const QStringList&);

		/** Automatically converts the \em ba to UTF-8.
		 */
		bool ParseMessage (const QString& ba);
		IrcMessageOptions GetIrcMessageOptions () const;
	};
}
