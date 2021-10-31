/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QStringList>
#include <QPair>
#include <QDateTime>
#include <QMetaType>

namespace LC::Azoth::Acetamide
{
	namespace Lits
	{
		extern const QString HumanReadableName;
		extern const QString StoredName;
		extern const QString Server;
		extern const QString Port;
		extern const QString ServerPassword;
		extern const QString Encoding;
		extern const QString Channel;
		extern const QString ChannelPassword;
		extern const QString Nickname;
		extern const QString SSL;
		extern const QString Autojoin;
		extern const QString AccountID;
		extern const QString PREFIX;

		extern const QString AzothAcetamide;
	}

	enum class IrcServer
	{
		UnknownServer,
		UnrealIRCD
	};

	struct ServerOptions
	{
		QString ServerName_;
		QString ServerEncoding_;
		QString ServerPassword_;
		QString ServerNickName_;
		int ServerPort_;
		bool SSL_;

		IrcServer IrcServer_ = IrcServer::UnknownServer;
		QString IrcServerVersion_;
	};

	struct ChannelOptions
	{
		QString ServerName_;
		QString ChannelName_;
		QString ChannelPassword_;

		bool operator== (const ChannelOptions&) const = default;
	};
	uint qHash (const ChannelOptions&);

	struct ChannelsDiscoverInfo
	{
		QString ChannelName_;
		int UsersCount_;
		QString Topic_;
	};

	enum ConnectionState
	{
		Connected,
		InProgress,
		NotConnected
	};

	enum ChannelRole
	{
		Participant,
		Voiced,
		HalfOperator,
		Operator,
		Admin,
		Owner
	};

	enum ChannelManagment
	{
		Kick,
		BanByName,
		BanByDomain,
		BanByUserAndDomain,
		KickAndBan
	};

	struct IrcMessageOptions
	{
		QString Nick_;
		QString UserName_;
		QString Host_;
		QString Command_;
		QString Message_;
		QList<std::string> Parameters_;
	};

	struct IrcBookmark
	{
		QString Name_;
		QString ServerName_;
		QString ServerPassword_;
		QString ServerEncoding_;
		QString NickName_;
		QString ChannelName_;
		QString ChannelPassword_;
		int ServerPort_;
		bool SSL_;
		bool AutoJoin_;
	};

	struct ChannelModes
	{
		bool InviteMode_ = false;
		bool ModerateMode_ = false;
		bool BlockOutsideMessageMode_ = false;
		bool PrivateMode_ = false;
		bool SecretMode_ = false;
		bool ReOpMode_ = false;
		bool OnlyOpChangeTopicMode_ = false;
		QPair<bool, int> UserLimit_ { false, 0 };
		QPair<bool, QString> ChannelKey_ { false, {} };
	};

	struct NickServIdentify
	{
		QString Server_;
		QString Nick_;
		QString NickServNick_;
		QString AuthString_;
		QString AuthMessage_;

		bool operator== (const NickServIdentify&) const = default;
	};

	struct MainEntryInfo
	{
		QString Nick_;
		QString UserName_;
		QString Host_;
		QString RealName_;
		QString ServerName_;
	};

	struct WhoMessage : MainEntryInfo
	{
		QString Flags_;
		QString Channel_;
		bool IsAway_;
		int Jumps_;
		QString EndString_;
	};

	struct WhoIsMessage : MainEntryInfo
	{
		QStringList Channels_;
		QString IdentifyAs_;
		QString IdleTime_;
		QString AuthTime_;
		QString IrcOperator_;
		QString ServerCountry_;
		QString LoggedName_;
		QString Secure_;
		QString EndString_;
		QString IsRegistered_;
		QString Mail_;
		QString IsHelpOp_;
		QString ConnectedFrom_;
	};
}

Q_DECLARE_METATYPE (LC::Azoth::Acetamide::NickServIdentify)
