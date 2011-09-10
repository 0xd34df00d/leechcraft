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

#include "serverresponcemanager.h"
#include <boost/bind.hpp>
#include <QTextCodec>
#include "ircserverhandler.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	ServerResponceManager::ServerResponceManager (IrcServerHandler *ish) 
	: QObject (ish)
	, ISH_ (ish)
	{
		Init ();
	}

	void ServerResponceManager::DoAction (const QString& cmd, 
			const QString& nick, 
			const QList<std::string>& params, 
			const QString& msg)
	{
		if (cmd == "privmsg" && IsCTCPMessage (msg))
			Command2Action_ ["ctcp_rpl"] (nick, params, msg);
		else if (cmd == "notice" && IsCTCPMessage (msg))
			Command2Action_ ["ctcp_rqst"] (nick, params, msg);
		else if (Command2Action_.contains (cmd))
			Command2Action_ [cmd] (nick, params, msg);
	}

	void ServerResponceManager::Init ()
	{
		Command2Action_ ["join"] = boost::bind (&ServerResponceManager::GotJoin, 
				this, _1, _2, _3);
		Command2Action_ ["part"] = boost::bind (&ServerResponceManager::GotPart,
					this, _1, _2, _3);
		Command2Action_ ["quit"] = boost::bind (&ServerResponceManager::GotQuit,
					this, _1, _2, _3);
		Command2Action_ ["privmsg"] = boost::bind (&ServerResponceManager::GotPrivMsg,
					this, _1, _2, _3);
		Command2Action_ ["notice"] = boost::bind (&ServerResponceManager::GotNoticeMsg,
					this, _1, _2, _3);
		Command2Action_ ["nick"] = boost::bind (&ServerResponceManager::GotNick,
					this, _1, _2, _3);
		Command2Action_ ["ping"] = boost::bind (&ServerResponceManager::GotPing,
					this, _1, _2, _3);
		Command2Action_ ["topic"] = boost::bind (&ServerResponceManager::GotTopic,
					this, _1, _2, _3);
		Command2Action_ ["kick"] = boost::bind (&ServerResponceManager::GotKick,
					this, _1, _2, _3);
		Command2Action_ ["invite"] = boost::bind (&ServerResponceManager::GotInvitation,
					this, _1, _2, _3);
		Command2Action_ ["ctcp_rpl"] = boost::bind (&ServerResponceManager::GotCTCPReply,
					this, _1, _2, _3);
		Command2Action_ ["ctcp_rqst"] = boost::bind (&ServerResponceManager::GotCTCPRequestResult,
					this, _1, _2, _3);
		Command2Action_ ["331"] = boost::bind (&ServerResponceManager::GotTopic,
					this, _1, _2, _3);
		Command2Action_ ["332"] = boost::bind (&ServerResponceManager::GotTopic,
					this, _1, _2, _3);
		Command2Action_ ["341"] = boost::bind (&ServerResponceManager::ShowInviteMessage,
					this, _1, _2, _3);
		Command2Action_ ["353"] = boost::bind (&ServerResponceManager::GotNames,
					this, _1, _2, _3);
		Command2Action_ ["366"] = boost::bind (&ServerResponceManager::GotEndOfNames,
					this, _1, _2, _3);
		Command2Action_ ["301"] = boost::bind (&ServerResponceManager::GotAwayReply,
					this, _1, _2, _3);
		Command2Action_ ["305"] = boost::bind (&ServerResponceManager::GotSetAway,
					this, _1, _2, _3);
		Command2Action_ ["306"] = boost::bind (&ServerResponceManager::GotSetAway,
					this, _1, _2, _3);
		Command2Action_ ["302"] = boost::bind (&ServerResponceManager::GotUserHost,
					this, _1, _2, _3);
		Command2Action_ ["303"] = boost::bind (&ServerResponceManager::GotIson,
					this, _1, _2, _3);
		Command2Action_ ["311"] = boost::bind (&ServerResponceManager::GotWhoIsUser,
					this, _1, _2, _3);
		Command2Action_ ["312"] = boost::bind (&ServerResponceManager::GotWhoIsServer,
					this, _1, _2, _3);
		Command2Action_ ["313"] = boost::bind (&ServerResponceManager::GotWhoIsOperator,
					this, _1, _2, _3);
		Command2Action_ ["317"] = boost::bind (&ServerResponceManager::GotWhoIsIdle,
					this, _1, _2, _3);
		Command2Action_ ["318"] = boost::bind (&ServerResponceManager::GotEndOfWhoIs,
					this, _1, _2, _3);
		Command2Action_ ["319"] = boost::bind (&ServerResponceManager::GotWhoIsChannels,
					this, _1, _2, _3);
		Command2Action_ ["314"] = boost::bind (&ServerResponceManager::GotWhoWas,
					this, _1, _2, _3);
		Command2Action_ ["369"] = boost::bind (&ServerResponceManager::GotEndOfWhoWas,
					this, _1, _2, _3);
		Command2Action_ ["352"] = boost::bind (&ServerResponceManager::GotWho,
					this, _1, _2, _3);
		Command2Action_ ["315"] = boost::bind (&ServerResponceManager::GotEndOfWho,
					this, _1, _2, _3);
		Command2Action_ ["342"] = boost::bind (&ServerResponceManager::GotSummoning,
					this, _1, _2, _3);
		Command2Action_ ["351"] = boost::bind (&ServerResponceManager::GotVersion,
					this, _1, _2, _3);
		Command2Action_ ["364"] = boost::bind (&ServerResponceManager::GotLinks,
					this, _1, _2, _3);
		Command2Action_ ["365"] = boost::bind (&ServerResponceManager::GotEndOfLinks,
					this, _1, _2, _3);
		Command2Action_ ["371"] = boost::bind (&ServerResponceManager::GotInfo,
					this, _1, _2, _3);
		Command2Action_ ["374"] = boost::bind (&ServerResponceManager::GotEndOfInfo,
					this, _1, _2, _3);
		Command2Action_ ["372"] = boost::bind (&ServerResponceManager::GotMotd,
					this, _1, _2, _3);
		Command2Action_ ["375"] = boost::bind (&ServerResponceManager::GotMotd,
					this, _1, _2, _3);
		Command2Action_ ["376"] = boost::bind (&ServerResponceManager::GotEndOfMotd,
					this, _1, _2, _3);
		Command2Action_ ["422"] = boost::bind (&ServerResponceManager::GotMotd,
					this, _1, _2, _3);
		Command2Action_ ["381"] = boost::bind (&ServerResponceManager::GotYoureOper,
					this, _1, _2, _3);
		Command2Action_ ["382"] = boost::bind (&ServerResponceManager::GotRehash,
					this, _1, _2, _3);
		Command2Action_ ["391"] = boost::bind (&ServerResponceManager::GotTime,
					this, _1, _2, _3);
		Command2Action_ ["251"] = boost::bind (&ServerResponceManager::GotLuserOnlyMsg,
					this, _1, _2, _3);
		Command2Action_ ["252"] = boost::bind (&ServerResponceManager::GotLuserParamsWithMsg,
					this, _1, _2, _3);
		Command2Action_ ["253"] = boost::bind (&ServerResponceManager::GotLuserParamsWithMsg,
					this, _1, _2, _3);
		Command2Action_ ["254"] = boost::bind (&ServerResponceManager::GotLuserParamsWithMsg,
					this, _1, _2, _3);
		Command2Action_ ["255"] = boost::bind (&ServerResponceManager::GotLuserOnlyMsg,
					this, _1, _2, _3);
		Command2Action_ ["392"] = boost::bind (&ServerResponceManager::GotUsersStart,
					this, _1, _2, _3);
		Command2Action_ ["393"] = boost::bind (&ServerResponceManager::GotUsers,
					this, _1, _2, _3);
		Command2Action_ ["395"] = boost::bind (&ServerResponceManager::GotNoUser,
					this, _1, _2, _3);
		Command2Action_ ["394"] = boost::bind (&ServerResponceManager::GotEndOfUsers,
					this, _1, _2, _3);
		Command2Action_ ["200"] = boost::bind (&ServerResponceManager::GotTraceLink,
					this, _1, _2, _3);
		Command2Action_ ["201"] = boost::bind (&ServerResponceManager::GotTraceConnecting,
					this, _1, _2, _3);
		Command2Action_ ["202"] = boost::bind (&ServerResponceManager::GotTraceHandshake,
					this, _1, _2, _3);
		Command2Action_ ["203"] = boost::bind (&ServerResponceManager::GotTraceUnknown,
					this, _1, _2, _3);
		Command2Action_ ["204"] = boost::bind (&ServerResponceManager::GotTraceOperator,
					this, _1, _2, _3);
		Command2Action_ ["205"] = boost::bind (&ServerResponceManager::GotTraceUser,
					this, _1, _2, _3);
		Command2Action_ ["206"] = boost::bind (&ServerResponceManager::GotTraceServer,
					this, _1, _2, _3);
		Command2Action_ ["207"] = boost::bind (&ServerResponceManager::GotTraceService,
					this, _1, _2, _3);
		Command2Action_ ["208"] = boost::bind (&ServerResponceManager::GotTraceNewType,
					this, _1, _2, _3);
		Command2Action_ ["209"] = boost::bind (&ServerResponceManager::GotTraceClass,
					this, _1, _2, _3);
		Command2Action_ ["261"] = boost::bind (&ServerResponceManager::GotTraceLog,
					this, _1, _2, _3);
		Command2Action_ ["262"] = boost::bind (&ServerResponceManager::GotTraceEnd,
					this, _1, _2, _3);
		Command2Action_ ["211"] = boost::bind (&ServerResponceManager::GotStatsLinkInfo,
					this, _1, _2, _3);
		Command2Action_ ["212"] = boost::bind (&ServerResponceManager::GotStatsCommands,
					this, _1, _2, _3);
		Command2Action_ ["219"] = boost::bind (&ServerResponceManager::GotStatsEnd,
					this, _1, _2, _3);
		Command2Action_ ["242"] = boost::bind (&ServerResponceManager::GotStatsUptime,
					this, _1, _2, _3);
		Command2Action_ ["243"] = boost::bind (&ServerResponceManager::GotStatsOline,
					this, _1, _2, _3);
		Command2Action_ ["256"] = boost::bind (&ServerResponceManager::GotAdmineMe,
					this, _1, _2, _3);
		Command2Action_ ["257"] = boost::bind (&ServerResponceManager::GotAdminLoc1,
					this, _1, _2, _3);
		Command2Action_ ["258"] = boost::bind (&ServerResponceManager::GotAdminLoc2,
					this, _1, _2, _3);
		Command2Action_ ["259"] = boost::bind (&ServerResponceManager::GotAdminEmail,
					this, _1, _2, _3);
		Command2Action_ ["263"] = boost::bind (&ServerResponceManager::GotTryAgain,
					this, _1, _2, _3);
		Command2Action_ ["005"] = boost::bind (&ServerResponceManager::GotISupport,
					 this, _1, _2, _3);
		Command2Action_ ["mode"] = boost::bind (&ServerResponceManager::GotChannelMode,
					 this, _1, _2, _3);
		Command2Action_ ["367"] = boost::bind (&ServerResponceManager::GotBanList,
				 this, _1, _2, _3);
		Command2Action_ ["368"] = boost::bind (&ServerResponceManager::GotBanListEnd,
				 this, _1, _2, _3);
		Command2Action_ ["348"] = boost::bind (&ServerResponceManager::GotExceptList,
				 this, _1, _2, _3);
		Command2Action_ ["349"] = boost::bind (&ServerResponceManager::GotExceptListEnd,
				 this, _1, _2, _3);
		Command2Action_ ["346"] = boost::bind (&ServerResponceManager::GotInviteList,
				 this, _1, _2, _3);
		Command2Action_ ["347"] = boost::bind (&ServerResponceManager::GotInviteListEnd,
				 this, _1, _2, _3);
		Command2Action_ ["324"] = boost::bind (&ServerResponceManager::GotChannelModes,
				 this, _1, _2, _3);
	}

	bool ServerResponceManager::IsCTCPMessage (const QString& msg)
	{
		return msg.startsWith ('\001') && msg.endsWith ('\001');
	}

	void ServerResponceManager::GotJoin (const QString& nick, 
			const QList<std::string>& params, const QString& msg)
	{
		if (nick == ISH_->GetNickName ())
		{
			QString channel;
			if (!params.isEmpty ())
				channel = QString::fromUtf8 (params.first ().c_str ());
			else if (!msg.isEmpty ())
				channel = msg;

			ChannelOptions co;
			co.ChannelName_ = channel;
			co.ServerName_ = ISH_->GetServerOptions ().ServerName_.toLower ();
			co.ChannelPassword_ = QString ();
			ISH_->JoinedChannel (co);
			return;
		}

		ISH_->JoinParticipant (nick, msg);
	}

	void ServerResponceManager::GotPart (const QString& nick, 
			const QList<std::string>& params, const QString& msg)
	{
		if (params.isEmpty ())
			return;

		const QString channel = QString::fromUtf8 (params.first ().c_str ());
		if (nick == ISH_->GetNickName ())
		{
			ISH_->CloseChannel (channel);
			return;
		}

		ISH_->LeaveParticipant (nick, channel, msg);
	}

	void ServerResponceManager::GotQuit (const QString& nick, 
			const QList<std::string>& , const QString& msg)
	{
		
		if (nick == ISH_->GetNickName ())
		{
			ISH_->QuitServer ();
			return;
		}

		ISH_->QuitParticipant (nick, msg);
	}

	void ServerResponceManager::GotPrivMsg (const QString& nick, 
			const QList<std::string>& params, const QString& msg)
	{
		if (params.isEmpty ())
			return;

		const QString target = QString::fromUtf8 (params.first ().c_str ());
		ISH_->IncomingMessage (nick, target, msg);
	}

	void ServerResponceManager::GotNoticeMsg (const QString& nick, 
			const QList<std::string>&, const QString& msg)
	{
		ISH_->IncomingNoticeMessage (nick, msg);
	}

	void ServerResponceManager::GotNick (const QString& nick, 
			const QList<std::string>& , const QString& msg)
	{
		ISH_->ChangeNickname (nick, msg);
	}

	void ServerResponceManager::GotPing (const QString&,
			const QList<std::string>&, const QString& msg)
	{
		ISH_->PongMessage (msg);
	}

	void ServerResponceManager::GotTopic (const QString&, 
			const QList<std::string>& params, const QString& msg)
	{
		QString channel = QString::fromUtf8 (params.last ().c_str ());
		ISH_->GotTopic (channel, msg);
	}

	void ServerResponceManager::GotKick (const QString& nick, 
			const QList<std::string>& params, const QString& msg)
	{
		const QString channel = QString::fromUtf8 (params.first ().c_str ());
		const QString target = QString::fromUtf8 (params.last ().c_str ());
		if (nick == target)
			return;

		ISH_->KickUserFromChannel (nick, channel, target, msg);
	}

	void ServerResponceManager::GotInvitation (const QString& nick, 
			const QList<std::string>&, const QString& msg)
	{
		if (XmlSettingsManager::Instance ().property ("ShowInviteDialog").toBool ())
			XmlSettingsManager::Instance ().setProperty ("InviteActionByDefault", 0);

		if (!XmlSettingsManager::Instance ().property ("InviteActionByDefault").toInt ())
			ISH_->GotInvitation (nick, msg);
		else if (XmlSettingsManager::Instance ().property ("InviteActionByDefault").toInt () == 1)
			GotJoin (QString (), QList<std::string> (), msg);

		ISH_->ShowAnswer (nick + tr (" invites you to a channel ") + msg);
	}

	void ServerResponceManager::ShowInviteMessage (const QString&, 
			const QList<std::string>& params, const QString&)
	{
		if (params.count () < 3)
			return;
		QString msg = tr ("You invite ") + QString::fromUtf8 (params.at (1).c_str ()) +
				tr (" to a channel ") + QString::fromUtf8 (params.at (2).c_str ());
		ISH_->ShowAnswer (msg);
	}

	void ServerResponceManager::GotCTCPReply (const QString& nick, 
			const QList<std::string>& params, const QString& msg)
	{
		if (params.isEmpty ())
			return;

		if (msg.isEmpty ())
			return;
		
		QStringList ctcpList = msg.mid (1, msg.length () - 2).split (' ');
		if (ctcpList.isEmpty ())
			return;
		
		QString cmd;
		QString outputMessage;
		const QString version = QString ("%1 %2 %3").arg ("Acetamide",
				"2.0",
				"(C) 2011 by the LeechCraft team");
		const QDateTime currentDT = QDateTime::currentDateTime ();
		const QString firstPartOutput = QString ("%1 %2 - %3").arg ("Acetamide", 
				"2.0", 
				"http://www.leechcraft.org");
		const QString target = QString::fromUtf8 (params.last ().c_str ());

		if (ctcpList.at (0).toLower () == "version")
		{
			cmd = QString ("%1 %2%3").arg ("\001VERSION", 
					version, QChar ('\001'));
			outputMessage = tr ("Received request %1 from %2, sending response")
					.arg ("VERSION", nick);
		}
		else if (ctcpList.at (0).toLower () == "ping")
		{
			cmd = QString ("%1 %2%3").arg ("\001PING ", 
					QString::number (currentDT.toTime_t ()), QChar ('\001'));
			outputMessage = tr ("Received request %1 from %2, sending response")
					.arg ("PING", nick);
		}
		else if (ctcpList.at (0).toLower () == "time")
		{
			cmd = QString ("%1 %2%3").arg ("\001TIME", 
					currentDT.toString ("ddd MMM dd hh:mm:ss yyyy"), 
					QChar ('\001'));
			outputMessage = tr ("Received request %1 from %2, sending response")
					.arg ("TIME", nick);
		}
		else if (ctcpList.at (0).toLower () == "source")
		{
			cmd = QString ("%1 %2 %3").arg ("\001SOURCE", firstPartOutput, 
					QChar ('\001'));
			outputMessage = tr ("Received request %1 from %2, sending response")
					.arg ("SOURCE", nick);
		}
		else if (ctcpList.at (0).toLower () == "clientinfo")
		{
			cmd = QString ("%1 %2 - %3 %4 %5").arg ("\001CLIENTINFO", 
					firstPartOutput, "Supported tags:", 
					"VERSION PING TIME SOURCE CLIENTINFO", QChar ('\001'));
			outputMessage = tr ("Received request %1 from %2, sending response")
					.arg ("CLIENTINFO", nick);
		}
		else if (ctcpList.at (0).toLower () == "action")
		{
			QString mess = "/me " + QStringList (ctcpList.mid (1)).join (" ");
			ISH_->IncomingMessage (nick, target, mess);
			return;
		}

		if (outputMessage.isEmpty ())
			return;

		ISH_->CTCPReply (nick, cmd, outputMessage);
	}

	void ServerResponceManager::GotCTCPRequestResult (const QString& nick, 
			const QList<std::string>& params, const QString& msg)
	{
		if (QString::fromUtf8 (params.first ().c_str ()) != ISH_->GetNickName())
			return;

		if (msg.isEmpty ())
			return;

		const QStringList ctcpList = msg.mid (1, msg.length () - 2).split (' ');
		if (ctcpList.isEmpty ())
			return;

		const QString output = tr ("Received answer CTCP-%1 from %2: %3")
				.arg (ctcpList.at (0), nick,
						(QStringList (ctcpList.mid (1))).join (" "));
		ISH_->CTCPRequestResult (output);
	}

	void ServerResponceManager::GotNames (const QString&, 
			const QList<std::string>& params, const QString& msg)
	{
		const QString channel = QString::fromUtf8 (params.last ().c_str ());
		QStringList participants = msg.split (' ');
		ISH_->GotNames (channel, participants);
	}

	void ServerResponceManager::GotEndOfNames (const QString&, 
			const QList<std::string>& params, const QString&)
	{
		const QString channel = QString::fromUtf8 (params.last ().c_str ());
		ISH_->GotEndOfNames (channel);
	}

	void ServerResponceManager::GotAwayReply (const QString&, 
			const QList<std::string>& params, const QString& msg)
	{
		if (params.isEmpty ())
			return;
		const QString target = QString::fromUtf8 (params.last ().c_str ());
		ISH_->ShowAnswer (target + " " + msg);
	}

	void ServerResponceManager::GotSetAway (const QString&, 
			const QList<std::string>&, const QString& msg)
	{
		ISH_->ShowAnswer (msg);
	}

	void ServerResponceManager::GotUserHost (const QString&,
			const QList<std::string>&, const QString& msg)
	{
		Q_FOREACH (const QString& str, msg.split (' '))
		{
			const QString user = str.left (str.indexOf ('='));
			const QString host = str.mid (str.indexOf ('=') + 1);
			ISH_->ShowUserHost (user, host);
		}
	}

	void ServerResponceManager::GotIson (const QString&, 
			const QList<std::string>&, const QString& msg)
	{
		Q_FOREACH (const QString& str, msg.split (' '))
			ISH_->ShowIsUserOnServer (str);
	}

	void ServerResponceManager::GotWhoIsUser (const QString& server, 
			const QList<std::string>& params, const QString& msg)
	{
		if (params.count () < 4)
			return;

		ISH_->SetLongMessageState (true);
		const QString message = QString::fromUtf8 (params.at (1).c_str ()) +
				" - " + QString::fromUtf8 (params.at (2).c_str ()) + "@"
				+ QString::fromUtf8 (params.at (3).c_str ()) +
				" (" + msg + ")";
		ISH_->ShowWhoIsReply (message);
	}

	void ServerResponceManager::GotWhoIsServer (const QString&, 
			const QList<std::string>& params, const QString& msg)
	{
		if (params.count () < 3)
			return;

		ISH_->SetLongMessageState (true);
		QString message = QString::fromUtf8 (params.at (1).c_str ()) +
		tr (" connected via ") +
		QString::fromUtf8 (params.at (2).c_str ()) +
		" (" + msg + ")";
		ISH_->ShowWhoIsReply (message);
	}

	void ServerResponceManager::GotWhoIsOperator (const QString&, 
			const QList<std::string>& params, const QString& msg)
	{
		if (params.count () < 2)
			return;

		ISH_->ShowWhoIsReply (QString::fromUtf8 (params.at (1).c_str ()) + 
				" " + msg);
	}

	void ServerResponceManager::GotWhoIsIdle (const QString&, 
			const QList<std::string>& params, const QString& msg)
	{
		if (params.count () < 2)
			return;

		ISH_->SetLongMessageState (true);
		QString message = QString::fromUtf8 (params.at (0).c_str ()) +
				" " + QString::fromUtf8 (params.at (1).c_str ()) +
				" " + msg;
		ISH_->ShowWhoIsReply (message);
	}

	void ServerResponceManager::GotEndOfWhoIs (const QString&,
			const QList<std::string>&, const QString& msg)
	{
		ISH_->ShowWhoIsReply (tr ("End of WHOIS"));
		ISH_->SetLongMessageState (false);
	}

	void ServerResponceManager::GotWhoIsChannels (const QString&, 
			const QList<std::string>& params, const QString& msg)
	{
		if (params.count () < 2)
			return;

		ISH_->SetLongMessageState (true);
		QString message = QString::fromUtf8 (params.at (1).c_str ()) +
				tr (" on the channels : ") + msg;
		ISH_->ShowWhoIsReply (message);
	}

	void ServerResponceManager::GotWhoWas (const QString&, 
			const QList<std::string>& params, const QString& msg)
	{
		const QString message = QString::fromUtf8 (params.at (1).c_str ()) +
				" - " + QString::fromUtf8 (params.at (2).c_str ()) + "@"
				+ QString::fromUtf8 (params.at (3).c_str ()) +
				" (" + msg + ")";
		ISH_->SetLongMessageState (true);
		ISH_->ShowWhoWasReply (message);
	}

	void ServerResponceManager::GotEndOfWhoWas (const QString&, 
			const QList<std::string>&, const QString& msg)
	{
		ISH_->ShowWhoWasReply (tr ("End of WHOWAS"));
		ISH_->SetLongMessageState (false);
	}

	void ServerResponceManager::GotWho (const QString&, 
			const QList<std::string>& params, const QString& msg)
	{
		if (params.isEmpty ())
			return;

		const QString message = QString::fromUtf8 (params
				.at (params.count () - 1).c_str ()) +
				" - " + QString::fromUtf8 (params.at (2).c_str ()) + "@"
				+ QString::fromUtf8 (params.at (3).c_str ()) +
				" (" + msg.split (' ').at (1) + ")";
		ISH_->SetLongMessageState (true);
		ISH_->ShowWhoReply (message);
	}

	void ServerResponceManager::GotEndOfWho (const QString&, 
			const QList<std::string>&, const QString&)
	{
		ISH_->ShowWhoReply (tr ("End of WHO"));
		ISH_->SetLongMessageState (false);
	}

	void ServerResponceManager::GotSummoning (const QString&, 
			const QList<std::string>& params, const QString&)
	{
		if (params.count () < 2)
			return;
		ISH_->ShowAnswer (QString::fromUtf8 (params.at (1).c_str ()) + 
				tr (" summoning to IRC"));
	}

	void ServerResponceManager::GotVersion (const QString&, 
			const QList<std::string>& params, const QString& msg)
	{
		QString string;
		Q_FOREACH (std::string str, params)
			string.append(QString::fromUtf8 (str.c_str ()) + " ");
		ISH_->ShowAnswer (string + msg);
	}

	void ServerResponceManager::GotLinks (const QString&, 
			const QList<std::string>& params, const QString& msg)
	{
		QString str;
		for (int i = 0; i < params.count (); ++i)
			if (i)
				str.append(QString::fromUtf8 (params [i].c_str ()) + " ");
		ISH_->SetLongMessageState (true);
		ISH_->ShowLinksReply (str + msg);
	}

	void ServerResponceManager::GotEndOfLinks (const QString&, 
			const QList<std::string>&, const QString&)
	{
		ISH_->ShowLinksReply (tr ("End of LINKS"));
		ISH_->SetLongMessageState (false);
	}

	void ServerResponceManager::GotInfo (const QString&,
			const QList<std::string>&, const QString& msg)
	{
		ISH_->SetLongMessageState (true);
		ISH_->ShowInfoReply (msg);
	}

	void ServerResponceManager::GotEndOfInfo (const QString&,
			const QList<std::string>&, const QString&)
	{
		ISH_->ShowInfoReply (tr ("End of INFO"));
		ISH_->SetLongMessageState (false);
	}

	void ServerResponceManager::GotMotd (const QString&, 
			const QList<std::string>&, const QString& msg)
	{
		ISH_->SetLongMessageState (true);
		ISH_->ShowMotdReply (msg);
	}

	void ServerResponceManager::GotEndOfMotd (const QString&, 
			const QList<std::string>&, const QString&)
	{
		ISH_->ShowMotdReply (tr ("End of MOTD"));
		ISH_->SetLongMessageState (false);
	}

	void ServerResponceManager::GotYoureOper (const QString&, 
			const QList<std::string>&, const QString& msg)
	{
		ISH_->ShowAnswer (msg);
	}

	void ServerResponceManager::GotRehash (const QString&, 
			const QList<std::string>& params, const QString& msg)
	{
		if (params.isEmpty ())
			return;
		ISH_->ShowAnswer (QString::fromUtf8 (params.last ().c_str ()) + 
			" :" + msg);
	}

	void ServerResponceManager::GotTime (const QString&, 
			const QList<std::string>& params, const QString& msg)
	{
		if (params.isEmpty ())
			return;
		ISH_->ShowAnswer (QString::fromUtf8 (params.last ().c_str ()) + 
				" :" + msg);
	}

	void ServerResponceManager::GotLuserOnlyMsg (const QString&, 
			const QList<std::string>&, const QString& msg)
	{
		ISH_->ShowAnswer (msg);
	}

	void ServerResponceManager::GotLuserParamsWithMsg (const QString&, 
			const QList<std::string>& params, const QString& msg)
	{
		if (params.isEmpty ())
			return;
		ISH_->ShowAnswer (QString::fromUtf8 (params.last ().c_str ()) + ":" 
				+ msg);
	}

	void ServerResponceManager::GotUsersStart (const QString&,
			const QList<std::string>& , const QString& msg)
	{
		ISH_->SetLongMessageState (true);
		ISH_->ShowUsersReply (msg);
	}

	void ServerResponceManager::GotUsers (const QString&,
			const QList<std::string>& , const QString& msg)
	{
		ISH_->ShowUsersReply (msg);
	}

	void ServerResponceManager::GotNoUser (const QString&,
			const QList<std::string>& , const QString& msg)
	{
		ISH_->ShowUsersReply (msg);
	}

	void ServerResponceManager::GotEndOfUsers (const QString&,
			const QList<std::string>&, const QString&)
	{
		ISH_->ShowUsersReply (tr ("End of USERS"));
		ISH_->SetLongMessageState (false);
	}

	void ServerResponceManager::GotTraceLink (const QString&,
			const QList<std::string>& params, const QString&)
	{
		if (params.isEmpty ())
			return;
		QString message;
		Q_FOREACH (const std::string& str, params.mid (1))
			message += QString::fromUtf8 (str.c_str ()) + " ";
		ISH_->SetLongMessageState (true);
		ISH_->ShowTraceReply (message);
	}

	void ServerResponceManager::GotTraceConnecting (const QString&,
			const QList<std::string>& params, const QString&)
	{
		if (params.isEmpty ())
			return;
		QString message;
		Q_FOREACH (const std::string& str, params.mid (1))
			message += QString::fromUtf8 (str.c_str ()) + " ";
		ISH_->ShowAnswer (message);
	}

	void ServerResponceManager::GotTraceHandshake (const QString&,
			const QList<std::string>& params, const QString&)
	{
		if (params.isEmpty ())
			return;
		QString message;
		Q_FOREACH (const std::string& str, params.mid (1))
			message += QString::fromUtf8 (str.c_str ()) + " ";
		ISH_->ShowTraceReply (message);
	}

	void ServerResponceManager::GotTraceUnknown (const QString&,
			const QList<std::string>& params, const QString&)
	{
		if (params.isEmpty ())
			return;
		QString message;
		Q_FOREACH (const std::string& str, params.mid (1))
			message += QString::fromUtf8 (str.c_str ()) + " ";
		ISH_->ShowTraceReply (message);
	}

	void ServerResponceManager::GotTraceOperator (const QString&,
			const QList<std::string>& params, const QString&)
	{
		if (params.isEmpty ())
			return;
		QString message;
		Q_FOREACH (const std::string& str, params.mid (1))
			message += QString::fromUtf8 (str.c_str ()) + " ";
		ISH_->ShowTraceReply (message);
	}

	void ServerResponceManager::GotTraceUser (const QString&,
			const QList<std::string>& params, const QString&)
	{
		if (params.isEmpty ())
			return;
		QString message;
		Q_FOREACH (const std::string& str, params.mid (1))
			message += QString::fromUtf8 (str.c_str ()) + " ";
		ISH_->ShowTraceReply (message);
	}

	void ServerResponceManager::GotTraceServer (const QString&,
			const QList<std::string>& params, const QString&)
	{
		if (params.isEmpty ())
			return;
		QString message;
		Q_FOREACH (const std::string& str, params.mid (1))
			message += QString::fromUtf8 (str.c_str ()) + " ";
		ISH_->ShowTraceReply (message);
	}

	void ServerResponceManager::GotTraceService (const QString&,
			const QList<std::string>& params, const QString&)
	{
		if (params.isEmpty ())
			return;
		QString message;
		Q_FOREACH (const std::string& str, params.mid (1))
			message += QString::fromUtf8 (str.c_str ()) + " ";
		ISH_->ShowTraceReply (message);
	}

	void ServerResponceManager::GotTraceNewType (const QString&,
			const QList<std::string>& params, const QString&)
	{
		if (params.isEmpty ())
			return;
		QString message;
		Q_FOREACH (const std::string& str, params.mid (1))
			message += QString::fromUtf8 (str.c_str ()) + " ";
		ISH_->ShowTraceReply (message);
	}

	void ServerResponceManager::GotTraceClass (const QString&,
			const QList<std::string>& params, const QString&)
	{
		if (params.isEmpty ())
			return;
		QString message;
		Q_FOREACH (const std::string& str, params.mid (1))
			message += QString::fromUtf8 (str.c_str ()) + " ";
		ISH_->ShowTraceReply (message);
	}

	void ServerResponceManager::GotTraceLog (const QString&,
			const QList<std::string>& params, const QString&)
	{
		if (params.isEmpty ())
			return;
		QString message;
		Q_FOREACH (const std::string& str, params.mid (1))
			message += QString::fromUtf8 (str.c_str ()) + " ";
		ISH_->ShowTraceReply (message);
	}

	void ServerResponceManager::GotTraceEnd (const QString&,
			const QList<std::string>& params, const QString& msg)
	{
		if (params.isEmpty ())
			return;
		QString server = QString::fromUtf8 (params
				.at (params.count () - 1).c_str ());
		ISH_->ShowTraceReply (server + " " + msg);
		ISH_->SetLongMessageState (false);
	}

	void ServerResponceManager::GotStatsLinkInfo (const QString&,
			const QList<std::string>& params, const QString&)
	{
		if (params.isEmpty ())
			return;
		QString message;
		Q_FOREACH (const std::string& str, params.mid (1))
			message += QString::fromUtf8 (str.c_str ()) + " ";
		ISH_->SetLongMessageState (true);
		ISH_->ShowStatsReply (message);
	}

	void ServerResponceManager::GotStatsCommands (const QString&,
			const QList<std::string>& params, const QString&)
	{
		if (params.isEmpty ())
			return;
		QString message;
		Q_FOREACH (const std::string& str, params.mid (1))
			message += QString::fromUtf8 (str.c_str ()) + " ";
		ISH_->ShowStatsReply (message);
	}

	void ServerResponceManager::GotStatsEnd (const QString&,
			const QList<std::string>& params, const QString& msg)
	{
		if (params.isEmpty ())
			return;
		QString letter = QString::fromUtf8 (params
				.at (params.count () - 1).c_str ());
		ISH_->ShowStatsReply (letter + " " + msg);
		ISH_->SetLongMessageState (false);
	}

	void ServerResponceManager::GotStatsUptime (const QString&,
			const QList<std::string>& , const QString& msg)
	{
		ISH_->ShowStatsReply (msg);
	}

	void ServerResponceManager::GotStatsOline (const QString&,
			const QList<std::string>& params, const QString&)
	{
		if (params.isEmpty ())
			return;
		QString message;
		Q_FOREACH (const std::string& str, params.mid (1))
			message += QString::fromUtf8 (str.c_str ()) + " ";
		ISH_->ShowStatsReply (message);
	}

	void ServerResponceManager::GotAdmineMe (const QString&,
			const QList<std::string>& params, const QString& msg)
	{
		if (params.isEmpty ())
			return;
		ISH_->ShowAnswer (QString::fromUtf8 (params.last ().c_str ()) + ":" + msg);
	}

	void ServerResponceManager::GotAdminLoc1 (const QString&,
			const QList<std::string>&, const QString& msg)
	{
		ISH_->ShowAnswer (msg);
	}

	void ServerResponceManager::GotAdminLoc2 (const QString&,
			const QList<std::string>&, const QString& msg)
	{
		ISH_->ShowAnswer (msg);
	}

	void ServerResponceManager::GotAdminEmail (const QString&,
			const QList<std::string>&, const QString& msg)
	{
		ISH_->ShowAnswer (msg);
	}

	void ServerResponceManager::GotTryAgain (const QString&,
			const QList<std::string>& params, const QString& msg)
	{
		if (params.isEmpty ())
			return;
		QString cmd = QString::fromUtf8 (params.last ().c_str ());
		ISH_->ShowAnswer (cmd + ":" + msg);
	}

	void ServerResponceManager::GotISupport (const QString&, 
			const QList<std::string>& params, const QString& msg)
	{
		ISH_->JoinFromQueue ();
		
		QString result;
		Q_FOREACH (const std::string& param, params)
			result.append (QString::fromUtf8 (param.c_str ())).append (" ");
		result.append (":").append (msg);
		ISH_->ParserISupport (result);
		ISH_->ShowAnswer (result);
	}

	void ServerResponceManager::GotChannelMode (const QString& nick, 
			const QList<std::string>& params, const QString& msg)
	{
		if (params.isEmpty ())
			return;

		if (params.count () == 1 && 
				QString::fromUtf8 (params.first ().c_str ()) == ISH_->GetNickName ())
		{
			ISH_->ParseUserMode (QString::fromUtf8 (params.first ().c_str ()),
					msg);
			return;
		}

		const QString channel = QString::fromUtf8 (params.first ().c_str ());

		if (params.count () == 2)
			ISH_->ParseChanMode (channel, 
					QString::fromUtf8 (params.at (1).c_str ()));
		else if (params.count () == 3)
			ISH_->ParseChanMode (channel, 
					QString::fromUtf8 (params.at (1).c_str ()),
					QString::fromUtf8 (params.at (2).c_str ()));
	}

	void ServerResponceManager::GotChannelModes (const QString&,
			const QList<std::string>& params, const QString&)
	{
		const QString channel = QString::fromUtf8 (params.at (1).c_str ());

		if (params.count () == 3)
			ISH_->ParseChanMode (channel,
					QString::fromUtf8 (params.at (2).c_str ()));
		else if (params.count () == 4)
			ISH_->ParseChanMode (channel,
					QString::fromUtf8 (params.at (2).c_str ()),
					QString::fromUtf8 (params.at (3).c_str ()));
	}

	void ServerResponceManager::GotBanList (const QString&, 
			const QList<std::string>& params, const QString&)
	{
		const int count = params.count ();
		QString channel;
		QString nick;
		QString mask;
		QDateTime time;

		if (count > 2)
		{
			channel = QString::fromUtf8 (params.at (1).c_str ());
			mask = QString::fromUtf8 (params.at (2).c_str ());
		}

		if (count > 3)
		{
			QString name = QString::fromUtf8 (params.at (3).c_str ());
			nick = name.left (name.indexOf ('!'));
		}

		if (count > 4)
			time = QDateTime::fromTime_t (QString::fromUtf8 (params.at (4).c_str ()).toInt ());
		
		ISH_->SetLongMessageState (true);
		ISH_->ShowBanList (channel, mask, nick, time);
	}

	void ServerResponceManager::GotBanListEnd (const QString&, 
			const QList<std::string>&, const QString& msg)
	{
		ISH_->ShowBanListEnd (msg);
		ISH_->SetLongMessageState (false);
	}

	void ServerResponceManager::GotExceptList (const QString&, 
			const QList<std::string>& params, const QString&)
	{
		const int count = params.count ();
		QString channel;
		QString nick;
		QString mask;
		QDateTime time;
		
		if (count > 2)
		{
			channel = QString::fromUtf8 (params.at (1).c_str ());
			mask = QString::fromUtf8 (params.at (2).c_str ());
		}

		if (count > 3)
		{
			QString name = QString::fromUtf8 (params.at (3).c_str ());
			nick = name.left (name.indexOf ('!'));
		}

		if (count > 4)
			time = QDateTime::fromTime_t (QString::fromUtf8 (params.at (4).c_str ()).toInt ());
		
		ISH_->SetLongMessageState (true);
		ISH_->ShowExceptList (channel, mask, nick, time);
	}

	void ServerResponceManager::GotExceptListEnd (const QString&, 
			const QList<std::string>&, const QString& msg)
	{
		ISH_->ShowExceptListEnd (msg);
		ISH_->SetLongMessageState (false);
	}

	void ServerResponceManager::GotInviteList (const QString&, 
			const QList<std::string>& params, const QString&)
	{
		const int count = params.count ();
		QString channel;
		QString nick;
		QString mask;
		QDateTime time;
		
		if (count > 2)
		{
			channel = QString::fromUtf8 (params.at (1).c_str ());
			mask = QString::fromUtf8 (params.at (2).c_str ());
		}

		if (count > 3)
		{
			QString name = QString::fromUtf8 (params.at (3).c_str ());
			nick = name.left (name.indexOf ('!'));
		}

		if (count > 4)
			time = QDateTime::fromTime_t (QString::fromUtf8 (params.at (4).c_str ()).toInt ());
		
		ISH_->SetLongMessageState (true);
		ISH_->ShowInviteList (channel, mask, nick, time);
	}

	void ServerResponceManager::GotInviteListEnd (const QString&, 
			const QList<std::string>&, const QString& msg)
	{
		ISH_->ShowInviteListEnd (msg);
		ISH_->SetLongMessageState (false);
	}
}
}
}

