/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "namauth.h"
#include <QIcon>
#include <QMessageBox>
#include <QFile>
#include <interfaces/core/icoreproxy.h>
#include <util/threads/coro/future.h>
#include <util/threads/coro.h>
#include <util/db/consistencychecker.h>
#include <util/sll/qtutil.h>
#include "namhandler.h"
#include "sqlstoragebackend.h"

namespace LC::NamAuth
{
	void Plugin::Init (ICoreProxy_ptr)
	{
		StartChecks ();
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.NamAuth";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "NamAuth"_qs;
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Provides basic support for HTTP-level authentication.");
	}

	QIcon Plugin::GetIcon () const
	{
		return {};
	}

	Util::ContextTask<> Plugin::StartChecks ()
	{
		co_await Util::AddContextObject { *this };

		namespace CC = Util::ConsistencyChecker;

		const auto dbPath = SQLStorageBackend::GetDBPath ();
		const auto checkResult = co_await CC::Check (dbPath);
		if (checkResult.IsLeft ())
		{
			const auto recoverResult = co_await CC::RecoverWithUserInteraction (dbPath, GetName ());
			co_await WithHandler (recoverResult,
					[&] (auto)
					{
						QFile::rename (dbPath, dbPath + ".old");
						return Util::IgnoreLeft {};
					});
		}

		InitStorage (GetProxyHolder ());
	}

	void Plugin::InitStorage (const ICoreProxy_ptr& proxy)
	{
		const auto sb = new SQLStorageBackend;
		new NamHandler { sb, proxy->GetNetworkAccessManager () };
	}
}

LC_EXPORT_PLUGIN (leechcraft_namauth, LC::NamAuth::Plugin);
