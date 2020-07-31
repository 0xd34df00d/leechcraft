/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "servermanager.h"
#include <cstring>
#include <QStringList>
#include <QDir>
#include <QtDebug>
#include <Wt/WServer.h>
#include <util/xsd/addressesmodelmanager.h>
#include <util/sys/paths.h>
#include "aggregatorapp.h"
#include "xmlsettingsmanager.h"

namespace LC
{
namespace Aggregator
{
namespace WebAccess
{
	namespace
	{
		class ArgcGenerator
		{
			QStringList Parms_;
		public:
			inline ArgcGenerator ()
			: Parms_ ("/usr/local/lib/leechcraft")
			{
			}

			inline void AddParm (const QString& name, const QString& parm)
			{
				Parms_ << name << parm;
			}

			inline int GetArgc () const
			{
				return Parms_.size ();
			}

			inline char** GetArgv () const
			{
				char **result = new char* [GetArgc () + 1];
				int i = 0;
				for (const auto& parm : Parms_)
				{
					result [i] = new char [parm.size () + 1];
					std::strcpy (result [i], parm.toLatin1 ());
					++i;
				}
				result [i] = 0;

				return result;
			}
		};
	}

	ServerManager::ServerManager (IProxyObject *proxy,
			ICoreProxy_ptr coreProxy,
			Util::AddressesModelManager *manager)
	: Server_ { new Wt::WServer }
	, AddrMgr_ { manager }
	{
		Server_->addEntryPoint (Wt::EntryPointType::Application,
				[&] (const Wt::WEnvironment& we) { return std::make_unique<AggregatorApp> (proxy, coreProxy, we); });

		connect (AddrMgr_,
				SIGNAL (addressesChanged ()),
				this,
				SLOT (reconfigureServer ()));
		reconfigureServer ();
	}

	void ServerManager::reconfigureServer ()
	{
		const auto& addresses = AddrMgr_->GetAddresses ();
		qDebug () << Q_FUNC_INFO << "reconfiguring server at" << addresses;

		if (Server_->isRunning ())
		{
			try
			{
				qDebug () << Q_FUNC_INFO << "stopping the server...";
				Server_->stop ();
			}
			catch (const std::exception& e)
			{
				qWarning () << Q_FUNC_INFO
						<< e.what ();
			}
		}

		if (addresses.isEmpty ())
			return;

		ArgcGenerator gen;
		gen.AddParm ("--docroot", "/usr/share/Wt;/favicon.ico,/resources,/style");

		const auto& addr = addresses.value (0);
		gen.AddParm ("--http-address", addr.first);
		gen.AddParm ("--http-port", addr.second);
		Server_->setServerConfiguration (gen.GetArgc (), gen.GetArgv ());

		const auto& logPath = Util::CreateIfNotExists ("aggregator/webaccess")
				.filePath ("wt.log");
		Server_->logger ().setFile (logPath.toStdString ());

		try
		{
			Server_->start ();
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
					<< e.what ();
		}
	}
}
}
}
