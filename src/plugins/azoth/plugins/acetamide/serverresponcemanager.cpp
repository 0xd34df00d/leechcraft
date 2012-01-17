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

	void ServerResponceManager::DoAction (const IrcMessageOptions& opts)
	{
		if (opts.Command_ == "privmsg" && IsCTCPMessage (opts.Message_))
			Command2Action_ ["ctcp_rpl"] (opts);
		else if (opts.Command_ == "notice" && IsCTCPMessage (opts.Message_))
			Command2Action_ ["ctcp_rqst"] (opts);
		else if (Command2Action_.contains (opts.Command_))
			Command2Action_ [opts.Command_] (opts);
		else
			ISH_->ShowAnswer ("UNKNOWN CMD " + opts.Command_, opts.Message_);
	}

	void ServerResponceManager::Init ()
	{
		Command2Action_ ["join"] = boost::bind (&ServerResponceManager::GotJoin,
				this, _1);
		Command2Action_ ["part"] = boost::bind (&ServerResponceManager::GotPart,
				this, _1);
		Command2Action_ ["quit"] = boost::bind (&ServerResponceManager::GotQuit,
				this, _1);
		Command2Action_ ["privmsg"] = boost::bind (&ServerResponceManager::GotPrivMsg,
				this, _1);
		Command2Action_ ["notice"] = boost::bind (&ServerResponceManager::GotNoticeMsg,
				this, _1);
		Command2Action_ ["msg"] = boost::bind (&ServerResponceManager::GotNick,
				this, _1);
		Command2Action_ ["ping"] = boost::bind (&ServerResponceManager::GotPing,
				this, _1);
		Command2Action_ ["topic"] = boost::bind (&ServerResponceManager::GotTopic,
				this, _1);
		Command2Action_ ["kick"] = boost::bind (&ServerResponceManager::GotKick,
				this, _1);
		Command2Action_ ["invite"] = boost::bind (&ServerResponceManager::GotInvitation,
				this, _1);
		Command2Action_ ["ctcp_rpl"] = boost::bind (&ServerResponceManager::GotCTCPReply,
				this, _1);
		Command2Action_ ["ctcp_rqst"] = boost::bind (&ServerResponceManager::GotCTCPRequestResult,
				this, _1);
		Command2Action_ ["331"] = boost::bind (&ServerResponceManager::GotTopic,
				this, _1);
		Command2Action_ ["332"] = boost::bind (&ServerResponceManager::GotTopic,
				this, _1);
		Command2Action_ ["341"] = boost::bind (&ServerResponceManager::ShowInviteMessage,
				this, _1);
		Command2Action_ ["353"] = boost::bind (&ServerResponceManager::GotNames,
				this, _1);
		Command2Action_ ["366"] = boost::bind (&ServerResponceManager::GotEndOfNames,
				this, _1);
		Command2Action_ ["301"] = boost::bind (&ServerResponceManager::GotAwayReply,
				this, _1);
		Command2Action_ ["305"] = boost::bind (&ServerResponceManager::GotSetAway,
				this, _1);
		Command2Action_ ["306"] = boost::bind (&ServerResponceManager::GotSetAway,
				this, _1);
		Command2Action_ ["302"] = boost::bind (&ServerResponceManager::GotUserHost,
				this, _1);
		Command2Action_ ["303"] = boost::bind (&ServerResponceManager::GotIson,
				this, _1);
		Command2Action_ ["311"] = boost::bind (&ServerResponceManager::GotWhoIsUser,
				this, _1);
		Command2Action_ ["312"] = boost::bind (&ServerResponceManager::GotWhoIsServer,
				this, _1);
		Command2Action_ ["313"] = boost::bind (&ServerResponceManager::GotWhoIsOperator,
				this, _1);
		Command2Action_ ["317"] = boost::bind (&ServerResponceManager::GotWhoIsIdle,
				this, _1);
		Command2Action_ ["318"] = boost::bind (&ServerResponceManager::GotEndOfWhoIs,
				this, _1);
		Command2Action_ ["319"] = boost::bind (&ServerResponceManager::GotWhoIsChannels,
				this, _1);
		Command2Action_ ["314"] = boost::bind (&ServerResponceManager::GotWhoWas,
				this, _1);
		Command2Action_ ["369"] = boost::bind (&ServerResponceManager::GotEndOfWhoWas,
				this, _1);
		Command2Action_ ["352"] = boost::bind (&ServerResponceManager::GotWho,
				this, _1);
		Command2Action_ ["315"] = boost::bind (&ServerResponceManager::GotEndOfWho,
				this, _1);
		Command2Action_ ["342"] = boost::bind (&ServerResponceManager::GotSummoning,
				this, _1);
		Command2Action_ ["351"] = boost::bind (&ServerResponceManager::GotVersion,
				this, _1);
		Command2Action_ ["364"] = boost::bind (&ServerResponceManager::GotLinks,
				this, _1);
		Command2Action_ ["365"] = boost::bind (&ServerResponceManager::GotEndOfLinks,
				this, _1);
		Command2Action_ ["371"] = boost::bind (&ServerResponceManager::GotInfo,
				this, _1);
		Command2Action_ ["374"] = boost::bind (&ServerResponceManager::GotEndOfInfo,
				this, _1);
		Command2Action_ ["372"] = boost::bind (&ServerResponceManager::GotMotd,
				this, _1);
		Command2Action_ ["375"] = boost::bind (&ServerResponceManager::GotMotd,
				this, _1);
		Command2Action_ ["376"] = boost::bind (&ServerResponceManager::GotEndOfMotd,
				this, _1);
		Command2Action_ ["422"] = boost::bind (&ServerResponceManager::GotMotd,
				this, _1);
		Command2Action_ ["381"] = boost::bind (&ServerResponceManager::GotYoureOper,
				this, _1);
		Command2Action_ ["382"] = boost::bind (&ServerResponceManager::GotRehash,
				this, _1);
		Command2Action_ ["391"] = boost::bind (&ServerResponceManager::GotTime,
				this, _1);
		Command2Action_ ["251"] = boost::bind (&ServerResponceManager::GotLuserOnlyMsg,
				this, _1);
		Command2Action_ ["252"] = boost::bind (&ServerResponceManager::GotLuserParamsWithMsg,
				this, _1);
		Command2Action_ ["253"] = boost::bind (&ServerResponceManager::GotLuserParamsWithMsg,
				this, _1);
		Command2Action_ ["254"] = boost::bind (&ServerResponceManager::GotLuserParamsWithMsg,
				this, _1);
		Command2Action_ ["255"] = boost::bind (&ServerResponceManager::GotLuserOnlyMsg,
				this, _1);
		Command2Action_ ["392"] = boost::bind (&ServerResponceManager::GotUsersStart,
				this, _1);
		Command2Action_ ["393"] = boost::bind (&ServerResponceManager::GotUsers,
				this, _1);
		Command2Action_ ["395"] = boost::bind (&ServerResponceManager::GotNoUser,
				this, _1);
		Command2Action_ ["394"] = boost::bind (&ServerResponceManager::GotEndOfUsers,
				this, _1);
		Command2Action_ ["200"] = boost::bind (&ServerResponceManager::GotTraceLink,
				this, _1);
		Command2Action_ ["201"] = boost::bind (&ServerResponceManager::GotTraceConnecting,
				this, _1);
		Command2Action_ ["202"] = boost::bind (&ServerResponceManager::GotTraceHandshake,
				this, _1);
		Command2Action_ ["203"] = boost::bind (&ServerResponceManager::GotTraceUnknown,
				this, _1);
		Command2Action_ ["204"] = boost::bind (&ServerResponceManager::GotTraceOperator,
				this, _1);
		Command2Action_ ["205"] = boost::bind (&ServerResponceManager::GotTraceUser,
				this, _1);
		Command2Action_ ["206"] = boost::bind (&ServerResponceManager::GotTraceServer,
				this, _1);
		Command2Action_ ["207"] = boost::bind (&ServerResponceManager::GotTraceService,
				this, _1);
		Command2Action_ ["208"] = boost::bind (&ServerResponceManager::GotTraceNewType,
				this, _1);
		Command2Action_ ["209"] = boost::bind (&ServerResponceManager::GotTraceClass,
				this, _1);
		Command2Action_ ["261"] = boost::bind (&ServerResponceManager::GotTraceLog,
				this, _1);
		Command2Action_ ["262"] = boost::bind (&ServerResponceManager::GotTraceEnd,
				this, _1);
		Command2Action_ ["211"] = boost::bind (&ServerResponceManager::GotStatsLinkInfo,
				this, _1);
		Command2Action_ ["212"] = boost::bind (&ServerResponceManager::GotStatsCommands,
				this, _1);
		Command2Action_ ["219"] = boost::bind (&ServerResponceManager::GotStatsEnd,
				this, _1);
		Command2Action_ ["242"] = boost::bind (&ServerResponceManager::GotStatsUptime,
				this, _1);
		Command2Action_ ["243"] = boost::bind (&ServerResponceManager::GotStatsOline,
				this, _1);
		Command2Action_ ["256"] = boost::bind (&ServerResponceManager::GotAdmineMe,
				this, _1);
		Command2Action_ ["257"] = boost::bind (&ServerResponceManager::GotAdminLoc1,
				this, _1);
		Command2Action_ ["258"] = boost::bind (&ServerResponceManager::GotAdminLoc2,
				this, _1);
		Command2Action_ ["259"] = boost::bind (&ServerResponceManager::GotAdminEmail,
				this, _1);
		Command2Action_ ["263"] = boost::bind (&ServerResponceManager::GotTryAgain,
				this, _1);
		Command2Action_ ["005"] = boost::bind (&ServerResponceManager::GotISupport,
				this, _1);
		Command2Action_ ["mode"] = boost::bind (&ServerResponceManager::GotChannelMode,
				this, _1);
		Command2Action_ ["367"] = boost::bind (&ServerResponceManager::GotBanList,
				 this, _1);
		Command2Action_ ["368"] = boost::bind (&ServerResponceManager::GotBanListEnd,
				 this, _1);
		Command2Action_ ["348"] = boost::bind (&ServerResponceManager::GotExceptList,
				 this, _1);
		Command2Action_ ["349"] = boost::bind (&ServerResponceManager::GotExceptListEnd,
				 this, _1);
		Command2Action_ ["346"] = boost::bind (&ServerResponceManager::GotInviteList,
				 this, _1);
		Command2Action_ ["347"] = boost::bind (&ServerResponceManager::GotInviteListEnd,
				 this, _1);
		Command2Action_ ["324"] = boost::bind (&ServerResponceManager::GotChannelModes,
				 this, _1);
	}

	bool ServerResponceManager::IsCTCPMessage (const QString& msg)
	{
		return msg.startsWith ('\001') && msg.endsWith ('\001');
	}

	void ServerResponceManager::GotJoin (const IrcMessageOptions& opts)
	{
		const QString& channel = opts.Message_.isEmpty () ?
				QString::fromUtf8 (opts.Parameters_.last ().c_str ()) :
				opts.Message_;

		if (opts.Nick_ == ISH_->GetNickName ())
		{
			ChannelOptions co;
			co.ChannelName_ = channel;
			co.ServerName_ = ISH_->GetServerOptions ().ServerName_.toLower ();
			co.ChannelPassword_ = QString ();
			ISH_->JoinedChannel (co);
		}
		else
			ISH_->JoinParticipant (opts.Nick_, channel, opts.Host_, opts.UserName_);
	}

	void ServerResponceManager::GotPart (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

        const QString channel (QString::fromUtf8 (opts.Parameters_.first ().c_str ()));
		if (opts.Nick_ == ISH_->GetNickName ())
		{
			ISH_->CloseChannel (channel);
			return;
		}

		ISH_->LeaveParticipant (opts.Nick_, channel, opts.Message_);
	}

	void ServerResponceManager::GotQuit (const IrcMessageOptions& opts)
	{
		if (opts.Nick_ == ISH_->GetNickName ())
			ISH_->QuitServer ();
		else
			ISH_->QuitParticipant (opts.Nick_, opts.Message_);
	}

	void ServerResponceManager::GotPrivMsg (const IrcMessageOptions& opts)
	{
        if (opts.Parameters_.isEmpty ())
			return;

        const QString target (QString::fromUtf8 (opts.Parameters_.first ().c_str ()));
		ISH_->IncomingMessage (opts.Nick_, target, opts.Message_);
	}

	void ServerResponceManager::GotNoticeMsg (const IrcMessageOptions& opts)
	{
		ISH_->IncomingNoticeMessage (opts.Nick_, opts.Message_);
	}

	void ServerResponceManager::GotNick (const IrcMessageOptions& opts)
	{
		ISH_->ChangeNickname (opts.Nick_, opts.Message_);
	}

	void ServerResponceManager::GotPing (const IrcMessageOptions& opts)
	{
		ISH_->PongMessage (opts.Message_);
	}

	void ServerResponceManager::GotTopic (const IrcMessageOptions& opts)
	{
        QString channel (QString::fromUtf8 (opts.Parameters_.last ().c_str ()));
		ISH_->GotTopic (channel, opts.Message_);
	}

	void ServerResponceManager::GotKick (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		const QString channel (QString::fromUtf8 (opts.Parameters_.first ().c_str ()));
		const QString target (QString::fromUtf8 (opts.Parameters_.last ().c_str ()));
		if (opts.Nick_ == target)
			return;

		ISH_->GotKickCommand (opts.Nick_, channel, target, opts.Message_);
	}

	void ServerResponceManager::GotInvitation (const IrcMessageOptions& opts)
	{
		if (XmlSettingsManager::Instance ().property ("ShowInviteDialog").toBool ())
			XmlSettingsManager::Instance ().setProperty ("InviteActionByDefault", 0);

		if (!XmlSettingsManager::Instance ().property ("InviteActionByDefault").toInt ())
			ISH_->GotInvitation (opts.Nick_, opts.Message_);
		else if (XmlSettingsManager::Instance ().property ("InviteActionByDefault").toInt () == 1)
			GotJoin (opts);

		ISH_->ShowAnswer ("invite", opts.Nick_ + tr (" invites you to a channel ") + opts.Message_);
	}

	void ServerResponceManager::ShowInviteMessage (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.count () < 3)
			return;

		QString msg = tr ("You invite ") + QString::fromUtf8 (opts.Parameters_.at (1).c_str ()) +
				tr (" to a channel ") + QString::fromUtf8 (opts.Parameters_.at (2).c_str ());
		ISH_->ShowAnswer ("invite", msg);
	}

	void ServerResponceManager::GotCTCPReply (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		if (opts.Message_.isEmpty ())
			return;

		QStringList ctcpList = opts.Message_.mid (1, opts.Message_.length () - 2).split (' ');
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
		const QString target = QString::fromUtf8 (opts.Parameters_.last ().c_str ());

		if (ctcpList.at (0).toLower () == "version")
		{
			cmd = QString ("%1 %2%3").arg ("\001VERSION",
					version, QChar ('\001'));
			outputMessage = tr ("Received request %1 from %2, sending response")
					.arg ("VERSION", opts.Nick_);
		}
		else if (ctcpList.at (0).toLower () == "ping")
		{
			cmd = QString ("%1 %2%3").arg ("\001PING ",
					QString::number (currentDT.toTime_t ()), QChar ('\001'));
			outputMessage = tr ("Received request %1 from %2, sending response")
					.arg ("PING", opts.Nick_);
		}
		else if (ctcpList.at (0).toLower () == "time")
		{
			cmd = QString ("%1 %2%3").arg ("\001TIME",
					currentDT.toString ("ddd MMM dd hh:mm:ss yyyy"),
					QChar ('\001'));
			outputMessage = tr ("Received request %1 from %2, sending response")
					.arg ("TIME", opts.Nick_);
		}
		else if (ctcpList.at (0).toLower () == "source")
		{
			cmd = QString ("%1 %2 %3").arg ("\001SOURCE", firstPartOutput,
					QChar ('\001'));
			outputMessage = tr ("Received request %1 from %2, sending response")
					.arg ("SOURCE", opts.Nick_);
		}
		else if (ctcpList.at (0).toLower () == "clientinfo")
		{
			cmd = QString ("%1 %2 - %3 %4 %5").arg ("\001CLIENTINFO",
					firstPartOutput, "Supported tags:",
					"VERSION PING TIME SOURCE CLIENTINFO", QChar ('\001'));
			outputMessage = tr ("Received request %1 from %2, sending response")
					.arg ("CLIENTINFO", opts.Nick_);
		}
		else if (ctcpList.at (0).toLower () == "action")
		{
			QString mess = "/me " + QStringList (ctcpList.mid (1)).join (" ");
			ISH_->IncomingMessage (opts.Nick_, target, mess);
			return;
		}

		if (outputMessage.isEmpty ())
			return;

		ISH_->CTCPReply (opts.Nick_, cmd, outputMessage);
	}

	void ServerResponceManager::GotCTCPRequestResult (const IrcMessageOptions& opts)
	{
		if (QString::fromUtf8 (opts.Parameters_.first ().c_str ()) != ISH_->GetNickName ())
			return;

		if (opts.Message_.isEmpty ())
			return;

		const QStringList ctcpList = opts.Message_.mid (1, opts.Message_.length () - 2).split (' ');
		if (ctcpList.isEmpty ())
			return;

		const QString output = tr ("Received answer CTCP-%1 from %2: %3")
				.arg (ctcpList.at (0), opts.Nick_,
						(QStringList (ctcpList.mid (1))).join (" "));
		ISH_->CTCPRequestResult (output);
	}

	void ServerResponceManager::GotNames (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		const QString channel = QString::fromUtf8 (opts.Parameters_.last ().c_str ());
		const QStringList& participants = opts.Message_.split (' ');
		ISH_->GotNames (channel, participants);
	}

	void ServerResponceManager::GotEndOfNames (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		const QString channel = QString::fromUtf8 (opts.Parameters_.last ().c_str ());
		ISH_->GotEndOfNames (channel);
	}

	void ServerResponceManager::GotAwayReply (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		const QString target = QString::fromUtf8 (opts.Parameters_.last ().c_str ());
		ISH_->ShowAnswer ("away", target + " " + opts.Message_);
	}

	void ServerResponceManager::GotSetAway (const IrcMessageOptions& opts)
	{
		ISH_->ShowAnswer ("away", opts.Message_);
	}

	void ServerResponceManager::GotUserHost (const IrcMessageOptions& opts)
	{
		Q_FOREACH (const QString& str, opts.Message_.split (' '))
		{
			const QString user = str.left (str.indexOf ('='));
			const QString host = str.mid (str.indexOf ('=') + 1);
			ISH_->ShowUserHost (user, host);
		}
	}

	void ServerResponceManager::GotIson (const IrcMessageOptions& opts)
	{
		Q_FOREACH (const QString& str, opts.Message_.split (' '))
			ISH_->ShowIsUserOnServer (str);
	}

	void ServerResponceManager::GotWhoIsUser (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.count () < 4)
			return;

		const QString& message = QString::fromUtf8 (opts.Parameters_.at (1).c_str ()) +
				" - " + QString::fromUtf8 (opts.Parameters_.at (2).c_str ()) + "@"
				+ QString::fromUtf8 (opts.Parameters_.at (3).c_str ()) +
				" (" + opts.Message_ + ")";
		ISH_->ShowWhoIsReply (message);
	}

	void ServerResponceManager::GotWhoIsServer (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.count () < 3)
			return;

		const QString& message = QString::fromUtf8 (opts.Parameters_.at (1).c_str ()) +
				tr (" connected via ") +
		QString::fromUtf8 (opts.Parameters_.at (2).c_str ()) +
				" (" + opts.Message_ + ")";
		ISH_->ShowWhoIsReply (message);
	}

	void ServerResponceManager::GotWhoIsOperator (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.count () < 2)
			return;

		ISH_->ShowWhoIsReply (QString::fromUtf8 (opts.Parameters_.at (1).c_str ()) +
				" " + opts.Message_);
	}

	void ServerResponceManager::GotWhoIsIdle (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.count () < 2)
			return;

		const QString& message = QString::fromUtf8 (opts.Parameters_.at (0).c_str ()) +
				" " + QString::fromUtf8 (opts.Parameters_.at (1).c_str ()) +
				" " + opts.Message_;
		ISH_->ShowWhoIsReply (message);
	}

	void ServerResponceManager::GotEndOfWhoIs (const IrcMessageOptions& opts)
	{
		ISH_->ShowWhoIsReply (opts.Message_, true);
	}

	void ServerResponceManager::GotWhoIsChannels (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.count () < 2)
			return;

		QString message = QString::fromUtf8 (opts.Parameters_.at (1).c_str ()) +
				tr (" on the channels : ") + opts.Message_;
		ISH_->ShowWhoIsReply (message);
	}

	void ServerResponceManager::GotWhoWas (const IrcMessageOptions& opts)
	{
		const QString message = QString::fromUtf8 (opts.Parameters_.at (1).c_str ()) +
				" - " + QString::fromUtf8 (opts.Parameters_.at (2).c_str ()) + "@"
				+ QString::fromUtf8 (opts.Parameters_.at (3).c_str ()) +
				" (" + opts.Message_ + ")";
		ISH_->ShowWhoWasReply (message);
	}

	void ServerResponceManager::GotEndOfWhoWas (const IrcMessageOptions& opts)
	{
        ISH_->ShowWhoWasReply (opts.Message_, true);
	}

	void ServerResponceManager::GotWho (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		const QString message = QString::fromUtf8 (opts.Parameters_
				.at (opts.Parameters_.count () - 1).c_str ()) +
				" - " + QString::fromUtf8 (opts.Parameters_.at (2).c_str ()) + "@"
				+ QString::fromUtf8 (opts.Parameters_.at (3).c_str ()) +
				" (" + opts.Message_.split (' ').at (1) + ")";
		ISH_->ShowWhoReply (message);
	}

	void ServerResponceManager::GotEndOfWho (const IrcMessageOptions& opts)
	{
        ISH_->ShowWhoReply (opts.Message_, true);
	}

	void ServerResponceManager::GotSummoning (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.count () < 2)
			return;

		ISH_->ShowAnswer ("summon", QString::fromUtf8 (opts.Parameters_.at (1).c_str ()) +
				tr (" summoning to IRC"));
	}

	void ServerResponceManager::GotVersion (const IrcMessageOptions& opts)
	{
		QString string;
		Q_FOREACH (std::string str, opts.Parameters_)
			string.append (QString::fromUtf8 (str.c_str ()) + " ");
		ISH_->ShowAnswer ("version", string + opts.Message_);
	}

	void ServerResponceManager::GotLinks (const IrcMessageOptions& opts)
	{
		QString str;
		for (int i = 0; i < opts.Parameters_.count (); ++i)
			if (i)
				str.append (QString::fromUtf8 (opts.Parameters_ [i].c_str ()) + " ");
		ISH_->ShowLinksReply (str + opts.Message_);
	}

	void ServerResponceManager::GotEndOfLinks (const IrcMessageOptions& opts)
	{
        ISH_->ShowLinksReply (opts.Message_, true);
	}

	void ServerResponceManager::GotInfo (const IrcMessageOptions& opts)
	{
		ISH_->ShowInfoReply (opts.Message_);
	}

	void ServerResponceManager::GotEndOfInfo (const IrcMessageOptions& opts)
	{
        ISH_->ShowInfoReply (opts.Message_, true);
	}

	void ServerResponceManager::GotMotd (const IrcMessageOptions& opts)
	{
		ISH_->ShowMotdReply (opts.Message_);
	}

	void ServerResponceManager::GotEndOfMotd (const IrcMessageOptions& opts)
	{
        ISH_->ShowMotdReply (opts.Message_, true);
	}

	void ServerResponceManager::GotYoureOper (const IrcMessageOptions& opts)
	{
		ISH_->ShowAnswer ("oper", opts.Message_);
	}

	void ServerResponceManager::GotRehash (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		ISH_->ShowAnswer ("rehash", QString::fromUtf8 (opts.Parameters_.last ().c_str ()) +
			" :" + opts.Message_);
	}

	void ServerResponceManager::GotTime (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		ISH_->ShowAnswer ("time", QString::fromUtf8 (opts.Parameters_.last ().c_str ()) +
				" :" + opts.Message_);
	}

	void ServerResponceManager::GotLuserOnlyMsg (const IrcMessageOptions& opts)
	{
		ISH_->ShowAnswer ("luser", opts.Message_);
	}

	void ServerResponceManager::GotLuserParamsWithMsg (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		ISH_->ShowAnswer ("luser", QString::fromUtf8 (opts.Parameters_.last ().c_str ()) + ":"
				+ opts.Message_);
	}

	void ServerResponceManager::GotUsersStart (const IrcMessageOptions& opts)
	{
		ISH_->ShowUsersReply (opts.Message_);
	}

	void ServerResponceManager::GotUsers (const IrcMessageOptions& opts)
	{
		ISH_->ShowUsersReply (opts.Message_);
	}

	void ServerResponceManager::GotNoUser (const IrcMessageOptions& opts)
	{
		ISH_->ShowUsersReply (opts.Message_);
	}

	void ServerResponceManager::GotEndOfUsers (const IrcMessageOptions&)
	{
		ISH_->ShowUsersReply (tr ("End of USERS"), true);
	}

	void ServerResponceManager::GotTraceLink (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		QString message;
		Q_FOREACH (const std::string& str, opts.Parameters_.mid (1))
			message += QString::fromUtf8 (str.c_str ()) + " ";
		ISH_->ShowTraceReply (message);
	}

	void ServerResponceManager::GotTraceConnecting (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		QString message;
		Q_FOREACH (const std::string& str, opts.Parameters_.mid (1))
			message += QString::fromUtf8 (str.c_str ()) + " ";
		ISH_->ShowAnswer ("trace", message);
	}

	void ServerResponceManager::GotTraceHandshake (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		QString message;
		Q_FOREACH (const std::string& str, opts.Parameters_.mid (1))
			message += QString::fromUtf8 (str.c_str ()) + " ";
		ISH_->ShowTraceReply (message);
	}

	void ServerResponceManager::GotTraceUnknown (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		QString message;
		Q_FOREACH (const std::string& str, opts.Parameters_.mid (1))
			message += QString::fromUtf8 (str.c_str ()) + " ";
		ISH_->ShowTraceReply (message);
	}

	void ServerResponceManager::GotTraceOperator (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		QString message;
		Q_FOREACH (const std::string& str, opts.Parameters_.mid (1))
			message += QString::fromUtf8 (str.c_str ()) + " ";
		ISH_->ShowTraceReply (message);
	}

	void ServerResponceManager::GotTraceUser (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		QString message;
		Q_FOREACH (const std::string& str, opts.Parameters_.mid (1))
			message += QString::fromUtf8 (str.c_str ()) + " ";
		ISH_->ShowTraceReply (message);
	}

	void ServerResponceManager::GotTraceServer (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		QString message;
		Q_FOREACH (const std::string& str, opts.Parameters_.mid (1))
			message += QString::fromUtf8 (str.c_str ()) + " ";
		ISH_->ShowTraceReply (message);
	}

	void ServerResponceManager::GotTraceService (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		QString message;
		Q_FOREACH (const std::string& str, opts.Parameters_.mid (1))
			message += QString::fromUtf8 (str.c_str ()) + " ";
		ISH_->ShowTraceReply (message);
	}

	void ServerResponceManager::GotTraceNewType (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		QString message;
		Q_FOREACH (const std::string& str, opts.Parameters_.mid (1))
			message += QString::fromUtf8 (str.c_str ()) + " ";
		ISH_->ShowTraceReply (message);
	}

	void ServerResponceManager::GotTraceClass (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		QString message;
		Q_FOREACH (const std::string& str, opts.Parameters_.mid (1))
			message += QString::fromUtf8 (str.c_str ()) + " ";
		ISH_->ShowTraceReply (message);
	}

	void ServerResponceManager::GotTraceLog (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		QString message;
		Q_FOREACH (const std::string& str, opts.Parameters_.mid (1))
			message += QString::fromUtf8 (str.c_str ()) + " ";
		ISH_->ShowTraceReply (message);
	}

	void ServerResponceManager::GotTraceEnd (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		QString server = QString::fromUtf8 (opts.Parameters_
				.at (opts.Parameters_.count () - 1).c_str ());
		ISH_->ShowTraceReply (server + " " + opts.Message_, true);
	}

	void ServerResponceManager::GotStatsLinkInfo (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		QString message;
		Q_FOREACH (const std::string& str, opts.Parameters_.mid (1))
			message += QString::fromUtf8 (str.c_str ()) + " ";
		ISH_->ShowStatsReply (message);
	}

	void ServerResponceManager::GotStatsCommands (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		QString message;
		Q_FOREACH (const std::string& str, opts.Parameters_.mid (1))
			message += QString::fromUtf8 (str.c_str ()) + " ";
		ISH_->ShowStatsReply (message);
	}

	void ServerResponceManager::GotStatsEnd (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		QString letter = QString::fromUtf8 (opts.Parameters_
				.at (opts.Parameters_.count () - 1).c_str ());
		ISH_->ShowStatsReply (letter + " " + opts.Message_, true);
	}

	void ServerResponceManager::GotStatsUptime (const IrcMessageOptions& opts)
	{
		ISH_->ShowStatsReply (opts.Message_);
	}

	void ServerResponceManager::GotStatsOline (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		QString message;
		Q_FOREACH (const std::string& str, opts.Parameters_.mid (1))
			message += QString::fromUtf8 (str.c_str ()) + " ";
		ISH_->ShowStatsReply (message);
	}

	void ServerResponceManager::GotAdmineMe (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		ISH_->ShowAnswer ("admin", QString::fromUtf8 (opts.Parameters_.last ().c_str ()) + ":" + opts.Message_);
	}

	void ServerResponceManager::GotAdminLoc1 (const IrcMessageOptions& opts)
	{
		ISH_->ShowAnswer ("admin", opts.Message_);
	}

	void ServerResponceManager::GotAdminLoc2 (const IrcMessageOptions& opts)
	{
		ISH_->ShowAnswer ("admin", opts.Message_);
	}

	void ServerResponceManager::GotAdminEmail (const IrcMessageOptions& opts)
	{
		ISH_->ShowAnswer ("admin", opts.Message_);
	}

	void ServerResponceManager::GotTryAgain (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		QString cmd = QString::fromUtf8 (opts.Parameters_.last ().c_str ());
		ISH_->ShowAnswer ("error", cmd + ":" + opts.Message_);
	}

	void ServerResponceManager::GotISupport (const IrcMessageOptions& opts)
	{
		ISH_->JoinFromQueue ();

		QString result;
		Q_FOREACH (const std::string& param, opts.Parameters_)
			result.append (QString::fromUtf8 (param.c_str ())).append (" ");
		result.append (":").append (opts.Message_);
		ISH_->ParserISupport (result);
		ISH_->ShowAnswer ("mode", result);
	}

	void ServerResponceManager::GotChannelMode (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		if (opts.Parameters_.count () == 1 &&
				QString::fromUtf8 (opts.Parameters_.first ().c_str ()) == ISH_->GetNickName ())
		{
			ISH_->ParseUserMode (QString::fromUtf8 (opts.Parameters_.first ().c_str ()),
					opts.Message_);
			return;
		}

		const QString channel = QString::fromUtf8 (opts.Parameters_.first ().c_str ());

		if (opts.Parameters_.count () == 2)
			ISH_->ParseChanMode (channel,
					QString::fromUtf8 (opts.Parameters_.at (1).c_str ()));
		else if (opts.Parameters_.count () == 3)
			ISH_->ParseChanMode (channel,
					QString::fromUtf8 (opts.Parameters_.at (1).c_str ()),
					QString::fromUtf8 (opts.Parameters_.at (2).c_str ()));
	}

	void ServerResponceManager::GotChannelModes (const IrcMessageOptions& opts)
	{
		const QString channel = QString::fromUtf8 (opts.Parameters_.at (1).c_str ());

		if (opts.Parameters_.count () == 3)
			ISH_->ParseChanMode (channel,
					QString::fromUtf8 (opts.Parameters_.at (2).c_str ()));
		else if (opts.Parameters_.count () == 4)
			ISH_->ParseChanMode (channel,
					QString::fromUtf8 (opts.Parameters_.at (2).c_str ()),
					QString::fromUtf8 (opts.Parameters_.at (3).c_str ()));
	}

	void ServerResponceManager::GotBanList (const IrcMessageOptions& opts)
	{
		const int count = opts.Parameters_.count ();
		QString channel;
		QString nick;
		QString mask;
		QDateTime time;

		if (count > 2)
		{
			channel = QString::fromUtf8 (opts.Parameters_.at (1).c_str ());
			mask = QString::fromUtf8 (opts.Parameters_.at (2).c_str ());
		}

		if (count > 3)
		{
			QString name = QString::fromUtf8 (opts.Parameters_.at (3).c_str ());
			nick = name.left (name.indexOf ('!'));
		}

		if (count > 4)
			time = QDateTime::fromTime_t (QString::fromUtf8 (opts.Parameters_.at (4).c_str ()).toInt ());

		ISH_->ShowBanList (channel, mask, opts.Nick_, time);
	}

	void ServerResponceManager::GotBanListEnd (const IrcMessageOptions& opts)
	{
		ISH_->ShowBanListEnd (opts.Message_);
	}

	void ServerResponceManager::GotExceptList (const IrcMessageOptions& opts)
	{
		const int count = opts.Parameters_.count ();
		QString channel;
		QString nick;
		QString mask;
		QDateTime time;

		if (count > 2)
		{
			channel = QString::fromUtf8 (opts.Parameters_.at (1).c_str ());
			mask = QString::fromUtf8 (opts.Parameters_.at (2).c_str ());
		}

		if (count > 3)
		{
			QString name = QString::fromUtf8 (opts.Parameters_.at (3).c_str ());
			nick = name.left (name.indexOf ('!'));
		}

		if (count > 4)
			time = QDateTime::fromTime_t (QString::fromUtf8 (opts.Parameters_.at (4).c_str ()).toInt ());

		ISH_->ShowExceptList (channel, mask, opts.Nick_, time);
	}

	void ServerResponceManager::GotExceptListEnd (const IrcMessageOptions& opts)
	{
		ISH_->ShowExceptListEnd (opts.Message_);
	}

	void ServerResponceManager::GotInviteList (const IrcMessageOptions& opts)
	{
		const int count = opts.Parameters_.count ();
		QString channel;
		QString nick;
		QString mask;
		QDateTime time;

		if (count > 2)
		{
			channel = QString::fromUtf8 (opts.Parameters_.at (1).c_str ());
			mask = QString::fromUtf8 (opts.Parameters_.at (2).c_str ());
		}

		if (count > 3)
		{
			QString name = QString::fromUtf8 (opts.Parameters_.at (3).c_str ());
			nick = name.left (name.indexOf ('!'));
		}

		if (count > 4)
			time = QDateTime::fromTime_t (QString::fromUtf8 (opts.Parameters_.at (4).c_str ()).toInt ());

		ISH_->ShowInviteList (channel, mask, opts.Nick_, time);
	}

	void ServerResponceManager::GotInviteListEnd (const IrcMessageOptions& opts)
	{
		ISH_->ShowInviteListEnd (opts.Message_);
	}
}
}
}

