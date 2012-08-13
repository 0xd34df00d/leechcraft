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

#include "serverresponsemanager.h"
#include <boost/bind.hpp>
#include <QTextCodec>
#include <interfaces/core/icoreproxy.h>
#include <util/util.h>
#include "ircserverhandler.h"
#include "xmlsettingsmanager.h"
#include "core.h"
#include "ircaccount.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	ServerResponseManager::ServerResponseManager (IrcServerHandler *ish)
	: QObject (ish)
	, ISH_ (ish)
	{
		Init ();
	}

	void ServerResponseManager::DoAction (const IrcMessageOptions& opts)
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

	void ServerResponseManager::Init ()
	{
		Command2Action_ ["join"] = boost::bind (&ServerResponseManager::GotJoin,
				this, _1);
		Command2Action_ ["part"] = boost::bind (&ServerResponseManager::GotPart,
				this, _1);
		Command2Action_ ["quit"] = boost::bind (&ServerResponseManager::GotQuit,
				this, _1);
		Command2Action_ ["privmsg"] = boost::bind (&ServerResponseManager::GotPrivMsg,
				this, _1);
		Command2Action_ ["notice"] = boost::bind (&ServerResponseManager::GotNoticeMsg,
				this, _1);
		Command2Action_ ["nick"] = boost::bind (&ServerResponseManager::GotNick,
				this, _1);
		Command2Action_ ["ping"] = boost::bind (&ServerResponseManager::GotPing,
				this, _1);
		Command2Action_ ["topic"] = boost::bind (&ServerResponseManager::GotTopic,
				this, _1);
		Command2Action_ ["kick"] = boost::bind (&ServerResponseManager::GotKick,
				this, _1);
		Command2Action_ ["invite"] = boost::bind (&ServerResponseManager::GotInvitation,
				this, _1);
		Command2Action_ ["ctcp_rpl"] = boost::bind (&ServerResponseManager::GotCTCPReply,
				this, _1);
		Command2Action_ ["ctcp_rqst"] = boost::bind (&ServerResponseManager::GotCTCPRequestResult,
				this, _1);
		Command2Action_ ["331"] = boost::bind (&ServerResponseManager::GotTopic,
				this, _1);
		Command2Action_ ["332"] = boost::bind (&ServerResponseManager::GotTopic,
				this, _1);
		Command2Action_ ["341"] = boost::bind (&ServerResponseManager::ShowInviteMessage,
				this, _1);
		Command2Action_ ["353"] = boost::bind (&ServerResponseManager::GotNames,
				this, _1);
		Command2Action_ ["366"] = boost::bind (&ServerResponseManager::GotEndOfNames,
				this, _1);
		Command2Action_ ["301"] = boost::bind (&ServerResponseManager::GotAwayReply,
				this, _1);
		Command2Action_ ["305"] = boost::bind (&ServerResponseManager::GotSetAway,
				this, _1);
		Command2Action_ ["306"] = boost::bind (&ServerResponseManager::GotSetAway,
				this, _1);
		Command2Action_ ["302"] = boost::bind (&ServerResponseManager::GotUserHost,
				this, _1);
		Command2Action_ ["303"] = boost::bind (&ServerResponseManager::GotIson,
				this, _1);
		Command2Action_ ["311"] = boost::bind (&ServerResponseManager::GotWhoIsUser,
				this, _1);
		Command2Action_ ["312"] = boost::bind (&ServerResponseManager::GotWhoIsServer,
				this, _1);
		Command2Action_ ["313"] = boost::bind (&ServerResponseManager::GotWhoIsOperator,
				this, _1);
		Command2Action_ ["317"] = boost::bind (&ServerResponseManager::GotWhoIsIdle,
				this, _1);
		Command2Action_ ["318"] = boost::bind (&ServerResponseManager::GotEndOfWhoIs,
				this, _1);
		Command2Action_ ["319"] = boost::bind (&ServerResponseManager::GotWhoIsChannels,
				this, _1);
		Command2Action_ ["314"] = boost::bind (&ServerResponseManager::GotWhoWas,
				this, _1);
		Command2Action_ ["369"] = boost::bind (&ServerResponseManager::GotEndOfWhoWas,
				this, _1);
		Command2Action_ ["352"] = boost::bind (&ServerResponseManager::GotWho,
				this, _1);
		Command2Action_ ["315"] = boost::bind (&ServerResponseManager::GotEndOfWho,
				this, _1);
		Command2Action_ ["342"] = boost::bind (&ServerResponseManager::GotSummoning,
				this, _1);
		Command2Action_ ["351"] = boost::bind (&ServerResponseManager::GotVersion,
				this, _1);
		Command2Action_ ["364"] = boost::bind (&ServerResponseManager::GotLinks,
				this, _1);
		Command2Action_ ["365"] = boost::bind (&ServerResponseManager::GotEndOfLinks,
				this, _1);
		Command2Action_ ["371"] = boost::bind (&ServerResponseManager::GotInfo,
				this, _1);
		Command2Action_ ["374"] = boost::bind (&ServerResponseManager::GotEndOfInfo,
				this, _1);
		Command2Action_ ["372"] = boost::bind (&ServerResponseManager::GotMotd,
				this, _1);
		Command2Action_ ["375"] = boost::bind (&ServerResponseManager::GotMotd,
				this, _1);
		Command2Action_ ["376"] = boost::bind (&ServerResponseManager::GotEndOfMotd,
				this, _1);
		Command2Action_ ["422"] = boost::bind (&ServerResponseManager::GotMotd,
				this, _1);
		Command2Action_ ["381"] = boost::bind (&ServerResponseManager::GotYoureOper,
				this, _1);
		Command2Action_ ["382"] = boost::bind (&ServerResponseManager::GotRehash,
				this, _1);
		Command2Action_ ["391"] = boost::bind (&ServerResponseManager::GotTime,
				this, _1);
		Command2Action_ ["251"] = boost::bind (&ServerResponseManager::GotLuserOnlyMsg,
				this, _1);
		Command2Action_ ["252"] = boost::bind (&ServerResponseManager::GotLuserParamsWithMsg,
				this, _1);
		Command2Action_ ["253"] = boost::bind (&ServerResponseManager::GotLuserParamsWithMsg,
				this, _1);
		Command2Action_ ["254"] = boost::bind (&ServerResponseManager::GotLuserParamsWithMsg,
				this, _1);
		Command2Action_ ["255"] = boost::bind (&ServerResponseManager::GotLuserOnlyMsg,
				this, _1);
		Command2Action_ ["392"] = boost::bind (&ServerResponseManager::GotUsersStart,
				this, _1);
		Command2Action_ ["393"] = boost::bind (&ServerResponseManager::GotUsers,
				this, _1);
		Command2Action_ ["395"] = boost::bind (&ServerResponseManager::GotNoUser,
				this, _1);
		Command2Action_ ["394"] = boost::bind (&ServerResponseManager::GotEndOfUsers,
				this, _1);
		Command2Action_ ["200"] = boost::bind (&ServerResponseManager::GotTraceLink,
				this, _1);
		Command2Action_ ["201"] = boost::bind (&ServerResponseManager::GotTraceConnecting,
				this, _1);
		Command2Action_ ["202"] = boost::bind (&ServerResponseManager::GotTraceHandshake,
				this, _1);
		Command2Action_ ["203"] = boost::bind (&ServerResponseManager::GotTraceUnknown,
				this, _1);
		Command2Action_ ["204"] = boost::bind (&ServerResponseManager::GotTraceOperator,
				this, _1);
		Command2Action_ ["205"] = boost::bind (&ServerResponseManager::GotTraceUser,
				this, _1);
		Command2Action_ ["206"] = boost::bind (&ServerResponseManager::GotTraceServer,
				this, _1);
		Command2Action_ ["207"] = boost::bind (&ServerResponseManager::GotTraceService,
				this, _1);
		Command2Action_ ["208"] = boost::bind (&ServerResponseManager::GotTraceNewType,
				this, _1);
		Command2Action_ ["209"] = boost::bind (&ServerResponseManager::GotTraceClass,
				this, _1);
		Command2Action_ ["261"] = boost::bind (&ServerResponseManager::GotTraceLog,
				this, _1);
		Command2Action_ ["262"] = boost::bind (&ServerResponseManager::GotTraceEnd,
				this, _1);
		Command2Action_ ["211"] = boost::bind (&ServerResponseManager::GotStatsLinkInfo,
				this, _1);
		Command2Action_ ["212"] = boost::bind (&ServerResponseManager::GotStatsCommands,
				this, _1);
		Command2Action_ ["219"] = boost::bind (&ServerResponseManager::GotStatsEnd,
				this, _1);
		Command2Action_ ["242"] = boost::bind (&ServerResponseManager::GotStatsUptime,
				this, _1);
		Command2Action_ ["243"] = boost::bind (&ServerResponseManager::GotStatsOline,
				this, _1);
		Command2Action_ ["256"] = boost::bind (&ServerResponseManager::GotAdmineMe,
				this, _1);
		Command2Action_ ["257"] = boost::bind (&ServerResponseManager::GotAdminLoc1,
				this, _1);
		Command2Action_ ["258"] = boost::bind (&ServerResponseManager::GotAdminLoc2,
				this, _1);
		Command2Action_ ["259"] = boost::bind (&ServerResponseManager::GotAdminEmail,
				this, _1);
		Command2Action_ ["263"] = boost::bind (&ServerResponseManager::GotTryAgain,
				this, _1);
		Command2Action_ ["005"] = boost::bind (&ServerResponseManager::GotISupport,
				this, _1);
		Command2Action_ ["mode"] = boost::bind (&ServerResponseManager::GotChannelMode,
				this, _1);
		Command2Action_ ["367"] = boost::bind (&ServerResponseManager::GotBanList,
				 this, _1);
		Command2Action_ ["368"] = boost::bind (&ServerResponseManager::GotBanListEnd,
				 this, _1);
		Command2Action_ ["348"] = boost::bind (&ServerResponseManager::GotExceptList,
				 this, _1);
		Command2Action_ ["349"] = boost::bind (&ServerResponseManager::GotExceptListEnd,
				 this, _1);
		Command2Action_ ["346"] = boost::bind (&ServerResponseManager::GotInviteList,
				 this, _1);
		Command2Action_ ["347"] = boost::bind (&ServerResponseManager::GotInviteListEnd,
				 this, _1);
		Command2Action_ ["324"] = boost::bind (&ServerResponseManager::GotChannelModes,
				 this, _1);

		//not from rfc
		Command2Action_ ["330"] = boost::bind (&ServerResponseManager::GotWhoIsAccount,
				this, _1);
		Command2Action_ ["671"] = boost::bind (&ServerResponseManager::GotWhoIsSecure,
				this, _1);
		Command2Action_ ["328"] = boost::bind (&ServerResponseManager::GotChannelUrl,
				this, _1);
		Command2Action_ ["333"] = boost::bind (&ServerResponseManager::GotTopicWhoTime,
				this, _1);
		Command2Action_ ["004"] = boost::bind (&ServerResponseManager::GotServerInfo,
				this, _1);
		Command2Action_ ["307"] = [this] (const IrcMessageOptions& opts)
			{ ISH_->ShowAnswer ("307", opts.Message_); };
		Command2Action_ ["310"] = [this] (const IrcMessageOptions& opts)
			{ ISH_->ShowAnswer ("310", opts.Message_); };
		Command2Action_ ["320"] = [this] (const IrcMessageOptions& opts)
			{ ISH_->ShowAnswer ("320", opts.Message_); };
		Command2Action_ ["378"] = [this] (const IrcMessageOptions& opts)
			{ ISH_->ShowAnswer ("278", opts.Message_); };

		MatchString2Server_ ["unreal"] = IrcServer::UnrealIRCD;
	}

	bool ServerResponseManager::IsCTCPMessage (const QString& msg)
	{
		return msg.startsWith ('\001') && msg.endsWith ('\001');
	}

	void ServerResponseManager::GotJoin (const IrcMessageOptions& opts)
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

	void ServerResponseManager::GotPart (const IrcMessageOptions& opts)
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

	void ServerResponseManager::GotQuit (const IrcMessageOptions& opts)
	{
		if (opts.Nick_ == ISH_->GetNickName ())
			ISH_->DisconnectFromServer ();
		else
			ISH_->QuitParticipant (opts.Nick_, opts.Message_);
	}

	void ServerResponseManager::GotPrivMsg (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		const QString target (QString::fromUtf8 (opts.Parameters_.first ().c_str ()));
		ISH_->IncomingMessage (opts.Nick_, target, opts.Message_);
	}

	void ServerResponseManager::GotNoticeMsg (const IrcMessageOptions& opts)
	{
		ISH_->IncomingNoticeMessage (opts.Nick_, opts.Message_);
	}

	void ServerResponseManager::GotNick (const IrcMessageOptions& opts)
	{
		ISH_->ChangeNickname (opts.Nick_, opts.Message_);
	}

	void ServerResponseManager::GotPing (const IrcMessageOptions& opts)
	{
		ISH_->PongMessage (opts.Message_);
	}

	void ServerResponseManager::GotTopic (const IrcMessageOptions& opts)
	{
        QString channel (QString::fromUtf8 (opts.Parameters_.last ().c_str ()));
		ISH_->GotTopic (channel, opts.Message_);
	}

	void ServerResponseManager::GotKick (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		const QString channel (QString::fromUtf8 (opts.Parameters_.first ().c_str ()));
		const QString target (QString::fromUtf8 (opts.Parameters_.last ().c_str ()));
		if (opts.Nick_ == target)
			return;

		ISH_->GotKickCommand (opts.Nick_, channel, target, opts.Message_);
	}

	void ServerResponseManager::GotInvitation (const IrcMessageOptions& opts)
	{
		if (XmlSettingsManager::Instance ().property ("ShowInviteDialog").toBool ())
			XmlSettingsManager::Instance ().setProperty ("InviteActionByDefault", 0);

		if (!XmlSettingsManager::Instance ().property ("InviteActionByDefault").toInt ())
			ISH_->GotInvitation (opts.Nick_, opts.Message_);
		else if (XmlSettingsManager::Instance ().property ("InviteActionByDefault").toInt () == 1)
			GotJoin (opts);

		ISH_->ShowAnswer ("invite", opts.Nick_ + tr (" invites you to a channel ") + opts.Message_);
	}

	void ServerResponseManager::ShowInviteMessage (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.count () < 3)
			return;

		QString msg = tr ("You invite ") + QString::fromUtf8 (opts.Parameters_.at (1).c_str ()) +
				tr (" to a channel ") + QString::fromUtf8 (opts.Parameters_.at (2).c_str ());
		ISH_->ShowAnswer ("invite", msg);
	}

	void ServerResponseManager::GotCTCPReply (const IrcMessageOptions& opts)
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
		const QString& lcVer = Core::Instance ().GetProxy ()->GetVersion ();
		const QString version = QString ("LeechCraft %1 (Acetamide 2.0) "
					"(c) 2006-2012 LeechCraft team")
				.arg (lcVer);
		const QDateTime currentDT = QDateTime::currentDateTime ();
		const QString firstPartOutput = QString ("LeechCraft %1 (Acetamide 2.0) - "
					"http://leechcraft.org")
				.arg (lcVer);
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

	void ServerResponseManager::GotCTCPRequestResult (const IrcMessageOptions& opts)
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

	void ServerResponseManager::GotNames (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		const QString channel = QString::fromUtf8 (opts.Parameters_.last ().c_str ());
		const QStringList& participants = opts.Message_.split (' ');
		ISH_->GotNames (channel, participants);
	}

	void ServerResponseManager::GotEndOfNames (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		const QString channel = QString::fromUtf8 (opts.Parameters_.last ().c_str ());
		ISH_->GotEndOfNames (channel);
	}

	void ServerResponseManager::GotAwayReply (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		const QString& target = QString::fromUtf8 (opts.Parameters_.last ().c_str ());
		ISH_->IncomingMessage (target, target, QString ("[AWAY] %1 :%2")
				.arg (target, opts.Message_), IMessage::MTStatusMessage);
	}

	void ServerResponseManager::GotSetAway (const IrcMessageOptions& opts)
	{
		switch (opts.Command_.toInt ())
		{
		case 305:
			ISH_->ChangeAway (false);
			break;
		case 306:
			ISH_->ChangeAway (true, opts.Message_);
			break;
		}

		ISH_->ShowAnswer ("away", opts.Message_, IMessage::MTStatusMessage);
	}

	void ServerResponseManager::GotUserHost (const IrcMessageOptions& opts)
	{
		Q_FOREACH (const QString& str, opts.Message_.split (' '))
		{
			const QString user = str.left (str.indexOf ('='));
			const QString host = str.mid (str.indexOf ('=') + 1);
			ISH_->ShowUserHost (user, host);
		}
	}

	void ServerResponseManager::GotIson (const IrcMessageOptions& opts)
	{
		Q_FOREACH (const QString& str, opts.Message_.split (' '))
			ISH_->ShowIsUserOnServer (str);
	}

	void ServerResponseManager::GotWhoIsUser (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.count () < 4)
			return;

		WhoIsMessage msg;
		msg.Nick_ = QString::fromUtf8 (opts.Parameters_.at (1).c_str ());
		msg.UserName_ = QString::fromUtf8 (opts.Parameters_.at (2).c_str ());
		msg.Host_ = QString::fromUtf8 (opts.Parameters_.at (3).c_str ());
		msg.RealName_ = opts.Message_;
		ISH_->ShowWhoIsReply (msg);
	}

	void ServerResponseManager::GotWhoIsServer (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.count () < 3)
			return;

		WhoIsMessage msg;
		msg.Nick_ = QString::fromUtf8 (opts.Parameters_.at (1).c_str ());
		msg.ServerName_ = QString::fromUtf8 (opts.Parameters_.at (2).c_str ());
		msg.ServerCountry_ = opts.Message_;
		ISH_->ShowWhoIsReply (msg);
	}

	void ServerResponseManager::GotWhoIsOperator (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.count () < 2)
			return;

		WhoIsMessage msg;
		msg.Nick_ = QString::fromUtf8 (opts.Parameters_.at (1).c_str ());
		msg.IrcOperator_ = opts.Message_;
		ISH_->ShowWhoIsReply (msg);
	}

	void ServerResponseManager::GotWhoIsIdle (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.count () < 2)
			return;

		WhoIsMessage msg;
		msg.Nick_ = QString::fromUtf8 (opts.Parameters_.at (1).c_str ());
		msg.IdleTime_ = Util::MakeTimeFromLong (QString::fromUtf8 (opts
				.Parameters_.at (2).c_str ()).toULong ());
		msg.AuthTime_ = QDateTime::fromTime_t (QString::fromUtf8 (opts
				.Parameters_.at (3).c_str ()).toUInt ()).toString (Qt::TextDate);
		ISH_->ShowWhoIsReply (msg);
	}

	void ServerResponseManager::GotEndOfWhoIs (const IrcMessageOptions& opts)
	{
		WhoIsMessage msg;
		msg.Nick_ = QString::fromUtf8 (opts.Parameters_.at (1).c_str ());
		msg.EndString_ = opts.Message_;
		ISH_->ShowWhoIsReply (msg, true);
	}

	void ServerResponseManager::GotWhoIsChannels (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.count () < 2)
			return;

		WhoIsMessage msg;
		msg.Nick_ = QString::fromUtf8 (opts.Parameters_.at (1).c_str ());
		msg.Channels_ = opts.Message_.split (' ', QString::SkipEmptyParts);
		ISH_->ShowWhoIsReply (msg);
	}

	void ServerResponseManager::GotWhoWas (const IrcMessageOptions& opts)
	{
		const QString message = QString::fromUtf8 (opts.Parameters_.at (1).c_str ()) +
				" - " + QString::fromUtf8 (opts.Parameters_.at (2).c_str ()) + "@"
				+ QString::fromUtf8 (opts.Parameters_.at (3).c_str ()) +
				" (" + opts.Message_ + ")";
		ISH_->ShowWhoWasReply (message);
	}

	void ServerResponseManager::GotEndOfWhoWas (const IrcMessageOptions& opts)
	{
        ISH_->ShowWhoWasReply (opts.Message_, true);
	}

	void ServerResponseManager::GotWho (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		WhoMessage msg;
		msg.Channel_ = QString::fromUtf8 (opts.Parameters_.at (1).c_str ());
		msg.UserName_ = QString::fromUtf8 (opts.Parameters_.at (2).c_str ());
		msg.Host_ = QString::fromUtf8 (opts.Parameters_.at (3).c_str ());
		msg.ServerName_ = QString::fromUtf8 (opts.Parameters_.at (4).c_str ());
		msg.Nick_ = QString::fromUtf8 (opts.Parameters_.at (5).c_str ());
		int index = opts.Message_.indexOf (' ');
		msg.RealName_ = opts.Message_.mid (index);
		msg.Jumps_ = opts.Message_.left (index).toInt ();
		msg.Flags_ = QString::fromUtf8 (opts.Parameters_.at (6).c_str ());
		if (msg.Flags_.at (0) == 'H')
			msg.IsAway_ = false;
		else if (msg.Flags_.at (0) == 'G')
			msg.IsAway_ = true;
		ISH_->ShowWhoReply (msg);
	}

	void ServerResponseManager::GotEndOfWho (const IrcMessageOptions& opts)
	{
		WhoMessage msg;
		msg.Nick_ = QString::fromUtf8 (opts.Parameters_.at (1).c_str ());
		msg.EndString_ = opts.Message_;
		ISH_->ShowWhoReply (msg, true);
	}

	void ServerResponseManager::GotSummoning (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.count () < 2)
			return;

		ISH_->ShowAnswer ("summon", QString::fromUtf8 (opts.Parameters_.at (1).c_str ()) +
				tr (" summoning to IRC"));
	}

	void ServerResponseManager::GotVersion (const IrcMessageOptions& opts)
	{
		QString string;
		Q_FOREACH (std::string str, opts.Parameters_)
			string.append (QString::fromUtf8 (str.c_str ()) + " ");
		ISH_->ShowAnswer ("version", string + opts.Message_);
	}

	void ServerResponseManager::GotLinks (const IrcMessageOptions& opts)
	{
		QString str;
		for (int i = 0; i < opts.Parameters_.count (); ++i)
			if (i)
				str.append (QString::fromUtf8 (opts.Parameters_ [i].c_str ()) + " ");
		ISH_->ShowLinksReply (str + opts.Message_);
	}

	void ServerResponseManager::GotEndOfLinks (const IrcMessageOptions& opts)
	{
        ISH_->ShowLinksReply (opts.Message_, true);
	}

	void ServerResponseManager::GotInfo (const IrcMessageOptions& opts)
	{
		ISH_->ShowInfoReply (opts.Message_);
	}

	void ServerResponseManager::GotEndOfInfo (const IrcMessageOptions& opts)
	{
        ISH_->ShowInfoReply (opts.Message_, true);
	}

	void ServerResponseManager::GotMotd (const IrcMessageOptions& opts)
	{
		ISH_->ShowMotdReply (opts.Message_);
	}

	void ServerResponseManager::GotEndOfMotd (const IrcMessageOptions& opts)
	{
        ISH_->ShowMotdReply (opts.Message_, true);
	}

	void ServerResponseManager::GotYoureOper (const IrcMessageOptions& opts)
	{
		ISH_->ShowAnswer ("oper", opts.Message_);
	}

	void ServerResponseManager::GotRehash (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		ISH_->ShowAnswer ("rehash", QString::fromUtf8 (opts.Parameters_.last ().c_str ()) +
			" :" + opts.Message_);
	}

	void ServerResponseManager::GotTime (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		ISH_->ShowAnswer ("time", QString::fromUtf8 (opts.Parameters_.last ().c_str ()) +
				" :" + opts.Message_);
	}

	void ServerResponseManager::GotLuserOnlyMsg (const IrcMessageOptions& opts)
	{
		ISH_->ShowAnswer ("luser", opts.Message_);
	}

	void ServerResponseManager::GotLuserParamsWithMsg (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		ISH_->ShowAnswer ("luser", QString::fromUtf8 (opts.Parameters_.last ().c_str ()) + ":"
				+ opts.Message_);
	}

	void ServerResponseManager::GotUsersStart (const IrcMessageOptions& opts)
	{
		ISH_->ShowUsersReply (opts.Message_);
	}

	void ServerResponseManager::GotUsers (const IrcMessageOptions& opts)
	{
		ISH_->ShowUsersReply (opts.Message_);
	}

	void ServerResponseManager::GotNoUser (const IrcMessageOptions& opts)
	{
		ISH_->ShowUsersReply (opts.Message_);
	}

	void ServerResponseManager::GotEndOfUsers (const IrcMessageOptions&)
	{
		ISH_->ShowUsersReply (tr ("End of USERS"), true);
	}

	void ServerResponseManager::GotTraceLink (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		QString message;
		Q_FOREACH (const std::string& str, opts.Parameters_.mid (1))
			message += QString::fromUtf8 (str.c_str ()) + " ";
		ISH_->ShowTraceReply (message);
	}

	void ServerResponseManager::GotTraceConnecting (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		QString message;
		Q_FOREACH (const std::string& str, opts.Parameters_.mid (1))
			message += QString::fromUtf8 (str.c_str ()) + " ";
		ISH_->ShowAnswer ("trace", message);
	}

	void ServerResponseManager::GotTraceHandshake (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		QString message;
		Q_FOREACH (const std::string& str, opts.Parameters_.mid (1))
			message += QString::fromUtf8 (str.c_str ()) + " ";
		ISH_->ShowTraceReply (message);
	}

	void ServerResponseManager::GotTraceUnknown (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		QString message;
		Q_FOREACH (const std::string& str, opts.Parameters_.mid (1))
			message += QString::fromUtf8 (str.c_str ()) + " ";
		ISH_->ShowTraceReply (message);
	}

	void ServerResponseManager::GotTraceOperator (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		QString message;
		Q_FOREACH (const std::string& str, opts.Parameters_.mid (1))
			message += QString::fromUtf8 (str.c_str ()) + " ";
		ISH_->ShowTraceReply (message);
	}

	void ServerResponseManager::GotTraceUser (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		QString message;
		Q_FOREACH (const std::string& str, opts.Parameters_.mid (1))
			message += QString::fromUtf8 (str.c_str ()) + " ";
		ISH_->ShowTraceReply (message);
	}

	void ServerResponseManager::GotTraceServer (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		QString message;
		Q_FOREACH (const std::string& str, opts.Parameters_.mid (1))
			message += QString::fromUtf8 (str.c_str ()) + " ";
		ISH_->ShowTraceReply (message);
	}

	void ServerResponseManager::GotTraceService (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		QString message;
		Q_FOREACH (const std::string& str, opts.Parameters_.mid (1))
			message += QString::fromUtf8 (str.c_str ()) + " ";
		ISH_->ShowTraceReply (message);
	}

	void ServerResponseManager::GotTraceNewType (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		QString message;
		Q_FOREACH (const std::string& str, opts.Parameters_.mid (1))
			message += QString::fromUtf8 (str.c_str ()) + " ";
		ISH_->ShowTraceReply (message);
	}

	void ServerResponseManager::GotTraceClass (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		QString message;
		Q_FOREACH (const std::string& str, opts.Parameters_.mid (1))
			message += QString::fromUtf8 (str.c_str ()) + " ";
		ISH_->ShowTraceReply (message);
	}

	void ServerResponseManager::GotTraceLog (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		QString message;
		Q_FOREACH (const std::string& str, opts.Parameters_.mid (1))
			message += QString::fromUtf8 (str.c_str ()) + " ";
		ISH_->ShowTraceReply (message);
	}

	void ServerResponseManager::GotTraceEnd (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		QString server = QString::fromUtf8 (opts.Parameters_
				.at (opts.Parameters_.count () - 1).c_str ());
		ISH_->ShowTraceReply (server + " " + opts.Message_, true);
	}

	void ServerResponseManager::GotStatsLinkInfo (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		QString message;
		Q_FOREACH (const std::string& str, opts.Parameters_.mid (1))
			message += QString::fromUtf8 (str.c_str ()) + " ";
		ISH_->ShowStatsReply (message);
	}

	void ServerResponseManager::GotStatsCommands (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		QString message;
		Q_FOREACH (const std::string& str, opts.Parameters_.mid (1))
			message += QString::fromUtf8 (str.c_str ()) + " ";
		ISH_->ShowStatsReply (message);
	}

	void ServerResponseManager::GotStatsEnd (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		QString letter = QString::fromUtf8 (opts.Parameters_
				.at (opts.Parameters_.count () - 1).c_str ());
		ISH_->ShowStatsReply (letter + " " + opts.Message_, true);
	}

	void ServerResponseManager::GotStatsUptime (const IrcMessageOptions& opts)
	{
		ISH_->ShowStatsReply (opts.Message_);
	}

	void ServerResponseManager::GotStatsOline (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		QString message;
		Q_FOREACH (const std::string& str, opts.Parameters_.mid (1))
			message += QString::fromUtf8 (str.c_str ()) + " ";
		ISH_->ShowStatsReply (message);
	}

	void ServerResponseManager::GotAdmineMe (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		ISH_->ShowAnswer ("admin", QString::fromUtf8 (opts.Parameters_.last ().c_str ()) + ":" + opts.Message_);
	}

	void ServerResponseManager::GotAdminLoc1 (const IrcMessageOptions& opts)
	{
		ISH_->ShowAnswer ("admin", opts.Message_);
	}

	void ServerResponseManager::GotAdminLoc2 (const IrcMessageOptions& opts)
	{
		ISH_->ShowAnswer ("admin", opts.Message_);
	}

	void ServerResponseManager::GotAdminEmail (const IrcMessageOptions& opts)
	{
		ISH_->ShowAnswer ("admin", opts.Message_);
	}

	void ServerResponseManager::GotTryAgain (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		QString cmd = QString::fromUtf8 (opts.Parameters_.last ().c_str ());
		ISH_->ShowAnswer ("error", cmd + ":" + opts.Message_);
	}

	void ServerResponseManager::GotISupport (const IrcMessageOptions& opts)
	{
		ISH_->JoinFromQueue ();

		QString result;
		Q_FOREACH (const std::string& param, opts.Parameters_)
			result.append (QString::fromUtf8 (param.c_str ())).append (" ");
		result.append (":").append (opts.Message_);
		ISH_->ParserISupport (result);
		ISH_->ShowAnswer ("mode", result);
	}

	void ServerResponseManager::GotChannelMode (const IrcMessageOptions& opts)
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

	void ServerResponseManager::GotChannelModes (const IrcMessageOptions& opts)
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

	void ServerResponseManager::GotBanList (const IrcMessageOptions& opts)
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

	void ServerResponseManager::GotBanListEnd (const IrcMessageOptions& opts)
	{
		ISH_->ShowBanListEnd (opts.Message_);
	}

	void ServerResponseManager::GotExceptList (const IrcMessageOptions& opts)
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

	void ServerResponseManager::GotExceptListEnd (const IrcMessageOptions& opts)
	{
		ISH_->ShowExceptListEnd (opts.Message_);
	}

	void ServerResponseManager::GotInviteList (const IrcMessageOptions& opts)
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

	void ServerResponseManager::GotInviteListEnd (const IrcMessageOptions& opts)
	{
		ISH_->ShowInviteListEnd (opts.Message_);
	}

	void ServerResponseManager::GotServerInfo (const IrcMessageOptions& opts)
	{
		QStringList answer;
		std::transform (opts.Parameters_.begin (), opts.Parameters_.end (),
			std::back_inserter (answer),
			[] (decltype (opts.Parameters_.front ()) param)
				{ return QString::fromUtf8 (param.c_str ()); });
		ISH_->ShowAnswer ("myinfo", answer.join (" "));

		QString ircServer = QString::fromUtf8 (opts.Parameters_.at (2).c_str ());
		IrcServer server;
		auto serversKeys = MatchString2Server_.keys ();
		auto it = std::find_if (serversKeys.begin (),serversKeys.end (),
				[&ircServer] (decltype (serversKeys.front ()) key)
					{ return ircServer.contains (key, Qt::CaseInsensitive); });

		if (it == serversKeys.end ())
			return;

		server = MatchString2Server_ [*it];
		ISH_->SetIrcServerInfo (server, ircServer);

		switch (server)
		{
			case IrcServer::UnrealIRCD:
				Command2Action_ ["307"] = [this] (const IrcMessageOptions& opts)
					{
						WhoIsMessage msg;
						msg.Nick_ = QString::fromUtf8 (opts.Parameters_ [1].c_str ());
						msg.IsRegistered_ = opts.Message_;
						ISH_->ShowWhoIsReply (msg);
					};
				Command2Action_ ["310"] = [this] (const IrcMessageOptions& opts)
					{
						WhoIsMessage msg;
						msg.Nick_ = QString::fromUtf8 (opts.Parameters_ [1].c_str ());
						msg.IsHelpOp_ = opts.Message_;
						ISH_->ShowWhoIsReply (msg);
					};
				Command2Action_ ["320"] = [this] (const IrcMessageOptions& opts)
					{
						WhoIsMessage msg;
						msg.Nick_ = QString::fromUtf8 (opts.Parameters_ [1].c_str ());
						msg.Mail_ = opts.Message_;
						ISH_->ShowWhoIsReply (msg);
					};
				Command2Action_ ["378"] = [this] (const IrcMessageOptions& opts)
					{
						WhoIsMessage msg;
						msg.Nick_ = QString::fromUtf8 (opts.Parameters_ [1].c_str ());
						msg.ConnectedFrom_ = opts.Message_;
						ISH_->ShowWhoIsReply (msg);
					};
				break;
			default:
				break;
		}
	}

	void ServerResponseManager::GotWhoIsAccount (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.count () < 3)
			return;

		WhoIsMessage msg;
		msg.Nick_ = QString::fromUtf8 (opts.Parameters_.at (1).c_str ());
		msg.LoggedName_ = QString::fromUtf8 (opts.Parameters_.at (2).c_str ());
		ISH_->ShowWhoIsReply (msg);
	}

	void ServerResponseManager::GotWhoIsSecure (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.count () < 2)
			return;

		WhoIsMessage msg;
		msg.Nick_ = QString::fromUtf8 (opts.Parameters_.at (1).c_str ());
		msg.Secure_ = opts.Message_;
		ISH_->ShowWhoIsReply (msg);
	}

	void ServerResponseManager::GotChannelUrl (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.count () < 2)
			return;

		ISH_->GotChannelUrl (QString::fromUtf8 (opts.Parameters_.at (1).c_str ()),
				opts.Message_);
	}

	void ServerResponseManager::GotTopicWhoTime (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.count () < 4)
			return;

		ISH_->GotTopicWhoTime (QString::fromUtf8 (opts.Parameters_.at (1).c_str ()),
				QString::fromUtf8 (opts.Parameters_.at (2).c_str ()),
				QString::fromUtf8 (opts.Parameters_.at (3).c_str ()).toULongLong ());
	}


}
}
}

