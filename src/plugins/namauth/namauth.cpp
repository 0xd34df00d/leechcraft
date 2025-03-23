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

		const auto checker = Util::ConsistencyChecker::Create (SQLStorageBackend::GetDBPath (), GetName ());
		const auto result = co_await checker->StartCheck ();

		if (const auto error = result.MaybeLeft ())
		{
			const auto dumpResult = co_await (*error)->DumpReinit ();
			co_await WithHandler (dumpResult,
					[] (const Util::ConsistencyChecker::DumpError& err)
					{
						QMessageBox::critical (nullptr,
								"LeechCraft"_qs,
								tr ("Unable to recover the HTTP passwords database: %1.")
										.arg (err.Error_));

						const auto& path = SQLStorageBackend::GetDBPath ();
						QFile::copy (path, path + ".old");
						QFile::remove (path);
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
