/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "mucommands.h"
#include <QIcon>
#include <util/util.h>
#include <interfaces/azoth/iclentry.h>
#include <interfaces/azoth/iproxyobject.h>
#include "commands.h"
#include "presencecommand.h"
#include "openurlcommand.h"
#include "descparser.h"

namespace LC
{
namespace Azoth
{
namespace MuCommands
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Util::InstallTranslator ("azoth_mucommands");

		CoreProxy_ = proxy;
	}

	void Plugin::SecondInit ()
	{
		const DescParser descParser;

		Names_ = StaticCommand
		{
			{ "/names" },
			[this] (ICLEntry *e, const QString& t) { return HandleNames (AzothProxy_, e, t); }
		};
		descParser (Names_);

		ListUrls_ = StaticCommand
		{
			{ "/urls" },
			[this] (ICLEntry *e, const QString& t) { return ListUrls (AzothProxy_, e, t); }
		};
		descParser (ListUrls_);

		OpenUrl_ = StaticCommand
		{
			{ "/openurl" },
			[this] (ICLEntry *e, const QString& t)
				{ return OpenUrl (CoreProxy_, AzothProxy_, e, t, OnlyHandle); }
		};
		descParser (OpenUrl_);

		FetchUrl_ = StaticCommand
		{
			{ "/fetchurl" },
			[this] (ICLEntry *e, const QString& t)
				{ return OpenUrl (CoreProxy_, AzothProxy_, e, t, OnlyDownload); }
		};
		descParser (FetchUrl_);

		VCard_ = StaticCommand
		{
			{ "/vcard" },
			[this] (ICLEntry *e, const QString& t) { return ShowVCard (AzothProxy_, e, t); }
		};
		descParser (VCard_);

		Version_ = StaticCommand
		{
			{ "/version" },
			[this] (ICLEntry *e, const QString& t) { return ShowVersion (AzothProxy_, e, t); }
		};
		descParser (Version_);

		Time_ = StaticCommand
		{
			{ "/time" },
			[this] (ICLEntry *e, const QString& t) { return ShowTime (AzothProxy_, e, t); }
		};
		descParser (Time_);

		Disco_ = StaticCommand
		{
			{ "/disco" },
			[this] (ICLEntry *e, const QString& t) { return Disco (AzothProxy_, e, t); }
		};
		descParser (Disco_);

		ChangeNick_ = StaticCommand
		{
			{ "/nick" },
			[this] (ICLEntry *e, const QString& t) { return ChangeNick (AzothProxy_, e, t); }
		};
		descParser (ChangeNick_);

		ChangeSubject_ = StaticCommand
		{
			{ "/subject", "/topic" },
			[this] (ICLEntry *e, const QString& t) { return ChangeSubject (AzothProxy_, e, t); }
		};
		descParser (ChangeSubject_);

		JoinMuc_ = StaticCommand
		{
			{ "/join" },
			[this] (ICLEntry *e, const QString& t) { return JoinMuc (AzothProxy_, e, t); }
		};
		descParser (JoinMuc_);

		LeaveMuc_ = StaticCommand
		{
			{ "/leave", "/part" },
			[this] (ICLEntry *e, const QString& t) { return LeaveMuc (AzothProxy_, e, t); }
		};
		descParser (LeaveMuc_);

		RejoinMuc_ = StaticCommand
		{
			{ "/rejoin" },
			[this] (ICLEntry *e, const QString& t) { return RejoinMuc (AzothProxy_, e, t); }
		};
		descParser (RejoinMuc_);

		Ping_ = StaticCommand
		{
			{ "/ping" },
			[this] (ICLEntry *e, const QString& t) { return Ping (AzothProxy_, e, t); }
		};
		descParser (Ping_);

		Last_ = StaticCommand
		{
			{ "/last" },
			[this] (ICLEntry *e, const QString& t) { return Last (AzothProxy_, e, t); }
		};
		descParser (Last_);

		Invite_ = StaticCommand
		{
			{ "/invite" },
			[this] (ICLEntry *e, const QString& t) { return Invite (AzothProxy_, e, t); }
		};
		descParser (Invite_);

		Pm_ = StaticCommand
		{
			{ "/pm" },
			[this] (ICLEntry *e, const QString& t) { return Pm (AzothProxy_, e, t); }
		};
		descParser (Pm_);

		Whois_ = StaticCommand
		{
			{ "/whois" },
			[this] (ICLEntry *e, const QString& t) { return Whois (AzothProxy_, e, t); }
		};
		descParser (Whois_);

		ListPerms_ = StaticCommand
		{
			{ "/listperms" },
			[this] (ICLEntry *e, const QString& t) { return ListPerms (AzothProxy_, e, t); }
		};
		descParser (ListPerms_);

		SetPerm_ = StaticCommand
		{
			{ "/setperm" },
			[this] (ICLEntry *e, const QString& t) { return SetPerm (AzothProxy_, e, t); }
		};
		descParser (SetPerm_);

		Kick_ = StaticCommand
		{
			{ "/kick" },
			[this] (ICLEntry *e, const QString& t) { return Kick (AzothProxy_, e, t); }
		};
		descParser (Kick_);

		Ban_ = StaticCommand
		{
			{ "/ban" },
			[this] (ICLEntry *e, const QString& t) { return Ban (AzothProxy_, e, t); }
		};
		descParser (Ban_);

		Subst_ = StaticCommand
		{
			{ "/subst" },
			[this] (ICLEntry *e, const QString& t) { return Subst (AzothProxy_, e, t); }
		};
		descParser (Subst_);

		Presence_ = StaticCommand
		{
			{ "/presence" },
			[this] (ICLEntry *e, const QString& t) { return SetPresence (AzothProxy_, e, t); }
		};
		descParser (Presence_);

		ChatPresence_ = StaticCommand
		{
			{ "/chatpresence" },
			[this] (ICLEntry *e, const QString& t) { return SetDirectedPresence (AzothProxy_, e, t); }
		};
		descParser (ChatPresence_);
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Azoth.MuCommands";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Azoth MuCommands";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Provides some common commands, both for conferences and for private chats, for Azoth.");
	}

	QIcon Plugin::GetIcon () const
	{
		return {};
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Plugins.Azoth.Plugins.IGeneralPlugin";
		return result;
	}

	StaticCommands_t Plugin::GetStaticCommands (ICLEntry *entry)
	{
		switch (entry->GetEntryType ())
		{
		case ICLEntry::EntryType::MUC:
			return
			{
				Names_, ListUrls_, OpenUrl_, FetchUrl_, VCard_, Version_, Time_, Disco_, Invite_,
				ChangeNick_, ChangeSubject_, JoinMuc_, LeaveMuc_, RejoinMuc_, Ping_, Last_,
				ListPerms_, SetPerm_, Kick_, Ban_, Pm_, Whois_, Subst_, Presence_, ChatPresence_
			};
		case ICLEntry::EntryType::PrivateChat:
			return
			{
				ListUrls_, OpenUrl_, FetchUrl_, VCard_, Version_, Time_, Disco_, JoinMuc_, Ping_, Last_,
				Invite_, ListPerms_, SetPerm_, Whois_, Subst_, Presence_, ChatPresence_
			};
		default:
			return
			{
				ListUrls_, OpenUrl_, FetchUrl_, VCard_, Version_, Time_, Disco_, JoinMuc_, Ping_, Last_,
				Invite_, Subst_, Presence_, ChatPresence_
			};
		}
	}

	void Plugin::initPlugin (QObject *proxy)
	{
		AzothProxy_ = qobject_cast<IProxyObject*> (proxy);
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_azoth_mucommands, LC::Azoth::MuCommands::Plugin);
