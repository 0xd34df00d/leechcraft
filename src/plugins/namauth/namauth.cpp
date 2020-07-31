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
#include <interfaces/core/icoreproxy.h>
#include <util/util.h>
#include <util/sll/visitor.h>
#include <util/threads/futures.h>
#include <util/db/consistencychecker.h>
#include "namhandler.h"
#include "sqlstoragebackend.h"

namespace LC
{
namespace NamAuth
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Util::InstallTranslator ("namauth");

		const auto checker = Util::ConsistencyChecker::Create (SQLStorageBackend::GetDBPath (), GetName ());
		Util::Sequence (this, checker->StartCheck ()) >>
				[=] (const Util::ConsistencyChecker::CheckResult_t& result)
				{
					Util::Visit (result,
							[=] (Util::ConsistencyChecker::Succeeded) { InitStorage (proxy); },
							[=] (Util::ConsistencyChecker::Failed failed)
							{
								Util::Sequence (this, failed->DumpReinit ()) >>
										[=] (const Util::ConsistencyChecker::DumpResult_t& result)
										{
											Util::Visit (result,
													[=] (Util::ConsistencyChecker::DumpError err)
													{
														QMessageBox::critical (nullptr,
																tr ("LeechCraft"),
																tr ("Unable to recover the HTTP passwords database: %1.")
																		.arg (err.Error_));

														const auto& path = SQLStorageBackend::GetDBPath ();
														QFile::copy (path, path + ".old");
														QFile::remove (path);

														InitStorage (proxy);
													},
													[=] (Util::ConsistencyChecker::DumpFinished)
													{
														InitStorage (proxy);
													});
										};
							});
				};
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
		return "NamAuth";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Provides basic support for HTTP-level authentication.");
	}

	QIcon Plugin::GetIcon () const
	{
		return {};
	}

	void Plugin::InitStorage (const ICoreProxy_ptr& proxy)
	{
		const auto sb = new SQLStorageBackend;
		new NamHandler { sb, proxy->GetNetworkAccessManager () };
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_namauth, LC::NamAuth::Plugin);
