/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "serverresponsemanager.h"
#include <util/sll/prelude.h>
#include <util/sll/qtutil.h>
#include <util/sll/statichash.h>
#include <interfaces/core/icoreproxy.h>
#include <util/util.h>
#include "ircserverhandler.h"
#include "ircaccount.h"
#include "localtypes.h"
#include "xmlsettingsmanager.h"

namespace LC::Azoth::Acetamide
{
	namespace
	{
		template<typename T>
		using CmdImpl_t = void (T::*) (const IrcMessageOptions&);

		using Util::KVPair;

		constexpr auto SRMHash = Util::MakeStringHash<CmdImpl_t<ServerResponseManager>> (
					KVPair { "join", &ServerResponseManager::GotJoin },
					KVPair { "part", &ServerResponseManager::GotPart },
					KVPair { "quit", &ServerResponseManager::GotQuit },
					KVPair { "privmsg", &ServerResponseManager::GotPrivMsg },
					KVPair { "notice", &ServerResponseManager::GotNoticeMsg },
					KVPair { "nick", &ServerResponseManager::GotNick },
					KVPair { "ping", &ServerResponseManager::GotPing },
					KVPair { "topic", &ServerResponseManager::GotTopic },
					KVPair { "kick", &ServerResponseManager::GotKick },
					KVPair { "invite", &ServerResponseManager::GotInvitation },
					KVPair { "ctcp_rpl", &ServerResponseManager::GotCTCPReply },
					KVPair { "ctcp_rqst", &ServerResponseManager::GotCTCPRequestResult },
					KVPair { "mode", &ServerResponseManager::GotChannelMode },
					KVPair { "331", &ServerResponseManager::GotTopic },
					KVPair { "332", &ServerResponseManager::GotTopic },
					KVPair { "341", &ServerResponseManager::ShowInviteMessage },
					KVPair { "353", &ServerResponseManager::GotNames },
					KVPair { "366", &ServerResponseManager::GotEndOfNames },
					KVPair { "301", &ServerResponseManager::GotAwayReply },
					KVPair { "305", &ServerResponseManager::GotSetAway },
					KVPair { "306", &ServerResponseManager::GotSetAway },
					KVPair { "302", &ServerResponseManager::GotUserHost },
					KVPair { "303", &ServerResponseManager::GotIson },
					KVPair { "311", &ServerResponseManager::GotWhoIsUser },
					KVPair { "312", &ServerResponseManager::GotWhoIsServer },
					KVPair { "313", &ServerResponseManager::GotWhoIsOperator },
					KVPair { "317", &ServerResponseManager::GotWhoIsIdle },
					KVPair { "318", &ServerResponseManager::GotEndOfWhoIs },
					KVPair { "319", &ServerResponseManager::GotWhoIsChannels },
					KVPair { "314", &ServerResponseManager::GotWhoWas },
					KVPair { "369", &ServerResponseManager::GotEndOfWhoWas },
					KVPair { "352", &ServerResponseManager::GotWho },
					KVPair { "315", &ServerResponseManager::GotEndOfWho },
					KVPair { "342", &ServerResponseManager::GotSummoning },
					KVPair { "351", &ServerResponseManager::GotVersion },
					KVPair { "364", &ServerResponseManager::GotLinks },
					KVPair { "365", &ServerResponseManager::GotEndOfLinks },
					KVPair { "371", &ServerResponseManager::GotInfo },
					KVPair { "374", &ServerResponseManager::GotEndOfInfo },
					KVPair { "372", &ServerResponseManager::GotMotd },
					KVPair { "375", &ServerResponseManager::GotMotd },
					KVPair { "376", &ServerResponseManager::GotEndOfMotd },
					KVPair { "422", &ServerResponseManager::GotMotd },
					KVPair { "381", &ServerResponseManager::GotYoureOper },
					KVPair { "382", &ServerResponseManager::GotRehash },
					KVPair { "391", &ServerResponseManager::GotTime },
					KVPair { "251", &ServerResponseManager::GotLuserOnlyMsg },
					KVPair { "252", &ServerResponseManager::GotLuserParamsWithMsg },
					KVPair { "253", &ServerResponseManager::GotLuserParamsWithMsg },
					KVPair { "254", &ServerResponseManager::GotLuserParamsWithMsg },
					KVPair { "255", &ServerResponseManager::GotLuserOnlyMsg },
					KVPair { "392", &ServerResponseManager::GotUsersStart },
					KVPair { "393", &ServerResponseManager::GotUsers },
					KVPair { "395", &ServerResponseManager::GotNoUser },
					KVPair { "394", &ServerResponseManager::GotEndOfUsers },
					KVPair { "200", &ServerResponseManager::GotTraceLink },
					KVPair { "201", &ServerResponseManager::GotTraceConnecting },
					KVPair { "202", &ServerResponseManager::GotTraceHandshake },
					KVPair { "203", &ServerResponseManager::GotTraceUnknown },
					KVPair { "204", &ServerResponseManager::GotTraceOperator },
					KVPair { "205", &ServerResponseManager::GotTraceUser },
					KVPair { "206", &ServerResponseManager::GotTraceServer },
					KVPair { "207", &ServerResponseManager::GotTraceService },
					KVPair { "208", &ServerResponseManager::GotTraceNewType },
					KVPair { "209", &ServerResponseManager::GotTraceClass },
					KVPair { "261", &ServerResponseManager::GotTraceLog },
					KVPair { "262", &ServerResponseManager::GotTraceEnd },
					KVPair { "211", &ServerResponseManager::GotStatsLinkInfo },
					KVPair { "212", &ServerResponseManager::GotStatsCommands },
					KVPair { "219", &ServerResponseManager::GotStatsEnd },
					KVPair { "242", &ServerResponseManager::GotStatsUptime },
					KVPair { "243", &ServerResponseManager::GotStatsOline },
					KVPair { "256", &ServerResponseManager::GotAdmineMe },
					KVPair { "257", &ServerResponseManager::GotAdminLoc1 },
					KVPair { "258", &ServerResponseManager::GotAdminLoc2 },
					KVPair { "259", &ServerResponseManager::GotAdminEmail },
					KVPair { "263", &ServerResponseManager::GotTryAgain },
					KVPair { "005", &ServerResponseManager::GotISupport },
					KVPair { "367", &ServerResponseManager::GotBanList },
					KVPair { "368", &ServerResponseManager::GotBanListEnd },
					KVPair { "348", &ServerResponseManager::GotExceptList },
					KVPair { "349", &ServerResponseManager::GotExceptListEnd },
					KVPair { "346", &ServerResponseManager::GotInviteList },
					KVPair { "347", &ServerResponseManager::GotInviteListEnd },
					KVPair { "324", &ServerResponseManager::GotChannelModes },

					//not from rfc
					KVPair { "330", &ServerResponseManager::GotWhoIsAccount },
					KVPair { "671", &ServerResponseManager::GotWhoIsSecure },
					KVPair { "328", &ServerResponseManager::GotChannelUrl },
					KVPair { "333", &ServerResponseManager::GotTopicWhoTime },
					KVPair { "004", &ServerResponseManager::GotServerInfo }
				);

		constexpr auto ISHHash = Util::MakeStringHash<CmdImpl_t<IrcServerHandler>> (
					KVPair { "321", &IrcServerHandler::GotChannelsListBegin },
					KVPair { "322", &IrcServerHandler::GotChannelsList },
					KVPair { "323", &IrcServerHandler::GotChannelsListEnd }
				);

		using CustomCommand_t = void (*) (IrcServerHandler&, const IrcMessageOptions&);

		constexpr auto UnrealCustoms = Util::MakeStringHash<CustomCommand_t> (
					KVPair
					{
						"307",
						+[] (IrcServerHandler& ish, const IrcMessageOptions& opts)
						{
							WhoIsMessage msg;
							msg.Nick_ = QString::fromStdString (opts.Parameters_ [1]);
							msg.IsRegistered_ = opts.Message_;
							ish.ShowWhoIsReply (msg);
						}
					},
					KVPair
					{
						"310",
						+[] (IrcServerHandler& ish, const IrcMessageOptions& opts)
						{
							WhoIsMessage msg;
							msg.Nick_ = QString::fromStdString (opts.Parameters_ [1]);
							msg.IsHelpOp_ = opts.Message_;
							ish.ShowWhoIsReply (msg);
						}
					},
					KVPair
					{
						"320",
						+[] (IrcServerHandler& ish, const IrcMessageOptions& opts)
						{
							WhoIsMessage msg;
							msg.Nick_ = QString::fromStdString (opts.Parameters_ [1]);
							msg.Mail_ = opts.Message_;
							ish.ShowWhoIsReply (msg);
						}
					},
					KVPair
					{
						"378",
						+[] (IrcServerHandler& ish, const IrcMessageOptions& opts)
						{
							WhoIsMessage msg;
							msg.Nick_ = QString::fromStdString (opts.Parameters_ [1]);
							msg.ConnectedFrom_ = opts.Message_;
							ish.ShowWhoIsReply (msg);
						}
					}
				);

		constexpr auto MatchString2Server = std::to_array<std::pair<QStringView, IrcServer>> ({
					{ u"unreal", IrcServer::UnrealIRCD }
				});
	}

	ServerResponseManager::ServerResponseManager (IrcServerHandler *ish)
	: ISH_ { ish }
	{
	}

	void ServerResponseManager::DoAction (const IrcMessageOptions& opts)
	{
		auto cmdUtf8 = opts.Command_.toUtf8 ();
		if (cmdUtf8 == "privmsg" && IsCTCPMessage (opts.Message_))
			cmdUtf8 = "ctcp_rpl";
		else if (cmdUtf8 == "notice" && IsCTCPMessage (opts.Message_))
			cmdUtf8 = "ctcp_rqst";

		switch (ISH_->GetServerOptions ().IrcServer_)
		{
		case IrcServer::UnknownServer:
			break;
		case IrcServer::UnrealIRCD:
			if (const auto actor = UnrealCustoms (Util::AsStringView (cmdUtf8)))
				return actor (*ISH_, opts);
			break;
		}

		if (const auto actor = SRMHash (Util::AsStringView (cmdUtf8)))
			(this->*actor) (opts);
		else if (const auto actor = ISHHash (Util::AsStringView (cmdUtf8)))
			(ISH_->*actor) (opts);
		else
			ISH_->ShowAnswer (opts.Command_.toUtf8 (), opts.Message_);
	}

	bool ServerResponseManager::IsCTCPMessage (const QString& msg)
	{
		return msg.startsWith ('\001') && msg.endsWith ('\001');
	}

	void ServerResponseManager::GotJoin (const IrcMessageOptions& opts)
	{
		const QString& channel = opts.Message_.isEmpty () ?
				QString::fromStdString (opts.Parameters_.last ()) :
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
			ISH_->JoinParticipant (opts.Nick_, channel, opts.UserName_, opts.Host_);
	}

	void ServerResponseManager::GotPart (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

        const QString channel (QString::fromStdString (opts.Parameters_.first ()));
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

		const QString target (QString::fromStdString (opts.Parameters_.first ()));
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
        QString channel (QString::fromStdString (opts.Parameters_.last ()));
		ISH_->GotTopic (channel, opts.Message_);
	}

	void ServerResponseManager::GotKick (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		const QString channel (QString::fromStdString (opts.Parameters_.first ()));
		const QString target (QString::fromStdString (opts.Parameters_.last ()));
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

		ISH_->ShowAnswer ("invite",
				tr ("You invite %1 to channel %2")
					.arg (QString::fromStdString (opts.Parameters_.at (1)),
						  QString::fromStdString (opts.Parameters_.at (2))));
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
		const QString& lcVer = GetProxyHolder ()->GetVersion ();
		const QString version = QString ("LeechCraft %1 (Acetamide 2.0)").arg (lcVer);
		const QDateTime currentDT = QDateTime::currentDateTime ();
		const QString firstPartOutput = QString ("LeechCraft %1 (Acetamide 2.0) - "
					"https://leechcraft.org")
				.arg (lcVer);
		const QString target = QString::fromStdString (opts.Parameters_.last ());

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
					QString::number (currentDT.toSecsSinceEpoch ()), QChar ('\001'));
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
		if (QString::fromStdString (opts.Parameters_.first ()) != ISH_->GetNickName ())
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

		const QString channel = QString::fromStdString (opts.Parameters_.last ());
		const QStringList& participants = opts.Message_.split (' ');
		ISH_->GotNames (channel, participants);
	}

	void ServerResponseManager::GotEndOfNames (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		const QString channel = QString::fromStdString (opts.Parameters_.last ());
		ISH_->GotEndOfNames (channel);
	}

	void ServerResponseManager::GotAwayReply (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		const QString& target = QString::fromStdString (opts.Parameters_.last ());
		ISH_->IncomingMessage (target, target, QString ("[AWAY] %1 :%2")
				.arg (target, opts.Message_), IMessage::Type::StatusMessage);
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

		ISH_->ShowAnswer ("away", opts.Message_, true, IMessage::Type::StatusMessage);
	}

	void ServerResponseManager::GotUserHost (const IrcMessageOptions& opts)
	{
		for (const auto& str : opts.Message_.splitRef (' '))
		{
			const auto& user = str.left (str.indexOf ('='));
			const auto& host = str.mid (str.indexOf ('=') + 1);
			ISH_->ShowUserHost (user.toString (), host.toString ());
		}
	}

	void ServerResponseManager::GotIson (const IrcMessageOptions& opts)
	{
		for (const auto& str : opts.Message_.splitRef (' '))
			ISH_->ShowIsUserOnServer (str.toString ());
	}

	void ServerResponseManager::GotWhoIsUser (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.count () < 4)
			return;

		WhoIsMessage msg;
		msg.Nick_ = QString::fromStdString (opts.Parameters_.at (1));
		msg.UserName_ = QString::fromStdString (opts.Parameters_.at (2));
		msg.Host_ = QString::fromStdString (opts.Parameters_.at (3));
		msg.RealName_ = opts.Message_;
		ISH_->ShowWhoIsReply (msg);
	}

	void ServerResponseManager::GotWhoIsServer (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.count () < 3)
			return;

		WhoIsMessage msg;
		msg.Nick_ = QString::fromStdString (opts.Parameters_.at (1));
		msg.ServerName_ = QString::fromStdString (opts.Parameters_.at (2));
		msg.ServerCountry_ = opts.Message_;
		ISH_->ShowWhoIsReply (msg);
	}

	void ServerResponseManager::GotWhoIsOperator (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.count () < 2)
			return;

		WhoIsMessage msg;
		msg.Nick_ = QString::fromStdString (opts.Parameters_.at (1));
		msg.IrcOperator_ = opts.Message_;
		ISH_->ShowWhoIsReply (msg);
	}

	void ServerResponseManager::GotWhoIsIdle (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.count () < 2)
			return;

		WhoIsMessage msg;
		msg.Nick_ = QString::fromStdString (opts.Parameters_.at (1));
		msg.IdleTime_ = Util::MakeTimeFromLong (std::stol (opts.Parameters_.at (2)));
		msg.AuthTime_ = QDateTime::fromSecsSinceEpoch (std::stoul (opts.Parameters_.at (3))).toString (Qt::TextDate);
		ISH_->ShowWhoIsReply (msg);
	}

	void ServerResponseManager::GotEndOfWhoIs (const IrcMessageOptions& opts)
	{
		WhoIsMessage msg;
		msg.Nick_ = QString::fromStdString (opts.Parameters_.at (1));
		msg.EndString_ = opts.Message_;
		ISH_->ShowWhoIsReply (msg, true);
	}

	void ServerResponseManager::GotWhoIsChannels (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.count () < 2)
			return;

		WhoIsMessage msg;
		msg.Nick_ = QString::fromStdString (opts.Parameters_.at (1));
		msg.Channels_ = opts.Message_.split (' ', Qt::SkipEmptyParts);
		ISH_->ShowWhoIsReply (msg);
	}

	void ServerResponseManager::GotWhoWas (const IrcMessageOptions& opts)
	{
		const QString message = QString::fromStdString (opts.Parameters_.at (1)) +
				" - " + QString::fromStdString (opts.Parameters_.at (2)) + "@"
				+ QString::fromStdString (opts.Parameters_.at (3)) +
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
		msg.Channel_ = QString::fromStdString (opts.Parameters_.at (1));
		msg.UserName_ = QString::fromStdString (opts.Parameters_.at (2));
		msg.Host_ = QString::fromStdString (opts.Parameters_.at (3));
		msg.ServerName_ = QString::fromStdString (opts.Parameters_.at (4));
		msg.Nick_ = QString::fromStdString (opts.Parameters_.at (5));
		int index = opts.Message_.indexOf (' ');
		msg.RealName_ = opts.Message_.mid (index);
		msg.Jumps_ = opts.Message_.left (index).toInt ();
		msg.Flags_ = QString::fromStdString (opts.Parameters_.at (6));
		if (msg.Flags_.at (0) == 'H')
			msg.IsAway_ = false;
		else if (msg.Flags_.at (0) == 'G')
			msg.IsAway_ = true;
		ISH_->ShowWhoReply (msg);
	}

	void ServerResponseManager::GotEndOfWho (const IrcMessageOptions& opts)
	{
		WhoMessage msg;
		msg.Nick_ = QString::fromStdString (opts.Parameters_.at (1));
		msg.EndString_ = opts.Message_;
		ISH_->ShowWhoReply (msg, true);
	}

	void ServerResponseManager::GotSummoning (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.count () < 2)
			return;

		ISH_->ShowAnswer ("summon", QString::fromStdString (opts.Parameters_.at (1)) +
				tr (" summoning to IRC"));
	}

	namespace
	{
		QString BuildParamsStr (const QList<std::string>& params)
		{
			QString string;
			for (const auto& str : params)
				string.append (QString::fromStdString (str) + " ");
			return string;
		}

		template<int N>
		QString BuildParamsStr (const IrcMessageOptions& opts)
		{
			return BuildParamsStr (opts.Parameters_.mid (N));
		}

		template<>
		QString BuildParamsStr<0> (const IrcMessageOptions& opts)
		{
			return BuildParamsStr (opts.Parameters_);
		}
	}

	void ServerResponseManager::GotVersion (const IrcMessageOptions& opts)
	{
		ISH_->ShowAnswer ("version", BuildParamsStr<0> (opts) + opts.Message_);
	}

	void ServerResponseManager::GotLinks (const IrcMessageOptions& opts)
	{
		ISH_->ShowLinksReply (BuildParamsStr<1> (opts) + opts.Message_);
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

		ISH_->ShowAnswer ("rehash", QString::fromStdString (opts.Parameters_.last ()) +
			" :" + opts.Message_);
	}

	void ServerResponseManager::GotTime (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		ISH_->ShowAnswer ("time", QString::fromStdString (opts.Parameters_.last ()) +
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

		ISH_->ShowAnswer ("luser", QString::fromStdString (opts.Parameters_.last ()) + ":"
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

		ISH_->ShowTraceReply (BuildParamsStr<1> (opts));
	}

	void ServerResponseManager::GotTraceConnecting (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		ISH_->ShowAnswer ("trace", BuildParamsStr<1> (opts));
	}

	void ServerResponseManager::GotTraceHandshake (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		ISH_->ShowTraceReply (BuildParamsStr<1> (opts));
	}

	void ServerResponseManager::GotTraceUnknown (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		ISH_->ShowTraceReply (BuildParamsStr<1> (opts));
	}

	void ServerResponseManager::GotTraceOperator (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		ISH_->ShowTraceReply (BuildParamsStr<1> (opts));
	}

	void ServerResponseManager::GotTraceUser (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		ISH_->ShowTraceReply (BuildParamsStr<1> (opts));
	}

	void ServerResponseManager::GotTraceServer (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		ISH_->ShowTraceReply (BuildParamsStr<1> (opts));
	}

	void ServerResponseManager::GotTraceService (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		ISH_->ShowTraceReply (BuildParamsStr<1> (opts));
	}

	void ServerResponseManager::GotTraceNewType (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		ISH_->ShowTraceReply (BuildParamsStr<1> (opts));
	}

	void ServerResponseManager::GotTraceClass (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		ISH_->ShowTraceReply (BuildParamsStr<1> (opts));
	}

	void ServerResponseManager::GotTraceLog (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		ISH_->ShowTraceReply (BuildParamsStr<1> (opts));
	}

	void ServerResponseManager::GotTraceEnd (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		const auto& server = QString::fromStdString (opts.Parameters_.last ());
		ISH_->ShowTraceReply (server + " " + opts.Message_, true);
	}

	void ServerResponseManager::GotStatsLinkInfo (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		ISH_->ShowStatsReply (BuildParamsStr<1> (opts));
	}

	void ServerResponseManager::GotStatsCommands (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		ISH_->ShowStatsReply (BuildParamsStr<1> (opts));
	}

	void ServerResponseManager::GotStatsEnd (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		const auto& letter = QString::fromStdString (opts.Parameters_.last ());
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

		ISH_->ShowStatsReply (BuildParamsStr<1> (opts));
	}

	void ServerResponseManager::GotAdmineMe (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		ISH_->ShowAnswer ("admin", QString::fromStdString (opts.Parameters_.last ()) + ":" + opts.Message_);
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

		QString cmd = QString::fromStdString (opts.Parameters_.last ());
		ISH_->ShowAnswer ("error", cmd + ":" + opts.Message_);
	}

	void ServerResponseManager::GotISupport (const IrcMessageOptions& opts)
	{
		ISH_->JoinFromQueue ();

		auto result = BuildParamsStr<0> (opts);
		result.append (":").append (opts.Message_);
		ISH_->ParserISupport (result);
		ISH_->ShowAnswer ("mode", result);
	}

	void ServerResponseManager::GotChannelMode (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.isEmpty ())
			return;

		if (opts.Parameters_.count () == 1 &&
				QString::fromStdString (opts.Parameters_.first ()) == ISH_->GetNickName ())
		{
			ISH_->ParseUserMode (QString::fromStdString (opts.Parameters_.first ()),
					opts.Message_);
			return;
		}

		const QString channel = QString::fromStdString (opts.Parameters_.first ());

		if (opts.Parameters_.count () == 2)
			ISH_->ParseChanMode (channel,
					QString::fromStdString (opts.Parameters_.at (1)));
		else if (opts.Parameters_.count () == 3)
			ISH_->ParseChanMode (channel,
					QString::fromStdString (opts.Parameters_.at (1)),
					QString::fromStdString (opts.Parameters_.at (2)));
	}

	void ServerResponseManager::GotChannelModes (const IrcMessageOptions& opts)
	{
		const QString channel = QString::fromStdString (opts.Parameters_.at (1));

		if (opts.Parameters_.count () == 3)
			ISH_->ParseChanMode (channel,
					QString::fromStdString (opts.Parameters_.at (2)));
		else if (opts.Parameters_.count () == 4)
			ISH_->ParseChanMode (channel,
					QString::fromStdString (opts.Parameters_.at (2)),
					QString::fromStdString (opts.Parameters_.at (3)));
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
			channel = QString::fromStdString (opts.Parameters_.at (1));
			mask = QString::fromStdString (opts.Parameters_.at (2));
		}

		if (count > 3)
		{
			QString name = QString::fromStdString (opts.Parameters_.at (3));
			nick = name.left (name.indexOf ('!'));
		}

		if (count > 4)
			time = QDateTime::fromSecsSinceEpoch (std::stoi (opts.Parameters_.at (4)));

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
			channel = QString::fromStdString (opts.Parameters_.at (1));
			mask = QString::fromStdString (opts.Parameters_.at (2));
		}

		if (count > 3)
		{
			QString name = QString::fromStdString (opts.Parameters_.at (3));
			nick = name.left (name.indexOf ('!'));
		}

		if (count > 4)
			time = QDateTime::fromSecsSinceEpoch (std::stoi (opts.Parameters_.at (4)));

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
			channel = QString::fromStdString (opts.Parameters_.at (1));
			mask = QString::fromStdString (opts.Parameters_.at (2));
		}

		if (count > 3)
		{
			QString name = QString::fromStdString (opts.Parameters_.at (3));
			nick = name.left (name.indexOf ('!'));
		}

		if (count > 4)
			time = QDateTime::fromSecsSinceEpoch (std::stoi (opts.Parameters_.at (4)));

		ISH_->ShowInviteList (channel, mask, opts.Nick_, time);
	}

	void ServerResponseManager::GotInviteListEnd (const IrcMessageOptions& opts)
	{
		ISH_->ShowInviteListEnd (opts.Message_);
	}

	void ServerResponseManager::GotServerInfo (const IrcMessageOptions& opts)
	{
		ISH_->ShowAnswer ("myinfo",
				Util::Map (opts.Parameters_, &QString::fromStdString).join (" "));

		const auto& ircServer = QString::fromStdString (opts.Parameters_.at (2));
		const auto it = std::find_if (MatchString2Server.begin (), MatchString2Server.end (),
				[&ircServer] (const auto& key) { return ircServer.contains (key.first, Qt::CaseInsensitive); });
		const auto server = it == MatchString2Server.end () ? IrcServer::UnknownServer : it->second;
		ISH_->SetIrcServerInfo (server, ircServer);
	}

	void ServerResponseManager::GotWhoIsAccount (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.count () < 3)
			return;

		WhoIsMessage msg;
		msg.Nick_ = QString::fromStdString (opts.Parameters_.at (1));
		msg.LoggedName_ = QString::fromStdString (opts.Parameters_.at (2));
		ISH_->ShowWhoIsReply (msg);
	}

	void ServerResponseManager::GotWhoIsSecure (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.count () < 2)
			return;

		WhoIsMessage msg;
		msg.Nick_ = QString::fromStdString (opts.Parameters_.at (1));
		msg.Secure_ = opts.Message_;
		ISH_->ShowWhoIsReply (msg);
	}

	void ServerResponseManager::GotChannelUrl (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.count () < 2)
			return;

		ISH_->GotChannelUrl (QString::fromStdString (opts.Parameters_.at (1)),
				opts.Message_);
	}

	void ServerResponseManager::GotTopicWhoTime (const IrcMessageOptions& opts)
	{
		if (opts.Parameters_.count () < 4)
			return;

		ISH_->GotTopicWhoTime (QString::fromStdString (opts.Parameters_.at (1)),
				QString::fromStdString (opts.Parameters_.at (2)),
				std::stoll (opts.Parameters_.at (3)));
	}


}

