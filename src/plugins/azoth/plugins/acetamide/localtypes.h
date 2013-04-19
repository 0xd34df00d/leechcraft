/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 **********************************************************************/

#ifndef PLUGINS_AZOTH_PLUGINS_ACETAMIDE_LOCALTYPES_H
#define PLUGINS_AZOTH_PLUGINS_ACETAMIDE_LOCALTYPES_H

#include <QStringList>
#include <QPair>
#include <QDateTime>

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
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
		bool NickServIdentify_;

		IrcServer IrcServer_;
		QString IrcServerVersion_;
	};

	struct ChannelOptions
	{
		QString ServerName_;
		QString ChannelName_;
		QString ChannelPassword_;
	};

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
		ChannelModes ();

		bool InviteMode_;
		bool ModerateMode_;
		bool BlockOutsideMessageMode_;
		bool PrivateMode_;
		bool SecretMode_;
		bool ReOpMode_;
		bool OnlyOpChangeTopicMode_;
		QPair<bool, int> UserLimit_;
		QPair<bool, QString> ChannelKey_;
	};

	struct NickServIdentify
	{
		QString Server_;
		QString Nick_;
		QString NickServNick_;
		QString AuthString_;
		QString AuthMessage_;
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

	bool operator== (const ChannelOptions&, const ChannelOptions&);

	bool operator== (const NickServIdentify&, const NickServIdentify&);
};
};
};


#endif // PLUGINS_AZOTH_PLUGINS_ACETAMIDE_LOCALTYPES_H
