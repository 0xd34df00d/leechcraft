/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#include "servermanager.h"
#include <cstring>
#include <QStringList>
#include <Wt/WServer>
#include "aggregatorapp.h"

namespace LeechCraft
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
				Q_FOREACH (const QString& parm, Parms_)
				{
					result [i] = new char [parm.size ()];
					std::strcpy (result [i], parm.toLatin1 ());
					++i;
				}
				result [i] = 0;

				return result;
			}
		};
	}

	ServerManager::ServerManager (IProxyObject *proxy, ICoreProxy_ptr coreProxy)
	: AggProxy_ (proxy)
	, CoreProxy_ (coreProxy)
	, Server_ (new Wt::WServer ())
	{
		ArgcGenerator gen;
		gen.AddParm ("--docroot", "/usr/share/Wt;/favicon.ico,/resources,/style");
		gen.AddParm ("--http-address", "0.0.0.0");
		gen.AddParm ("--http-port", "9001");
		Server_->setServerConfiguration (gen.GetArgc (), gen.GetArgv ());
		Server_->addEntryPoint (Wt::Application,
				[proxy, coreProxy] (const Wt::WEnvironment& we)
					{ return new AggregatorApp (proxy, coreProxy, we); });
		Server_->start ();
	}
}
}
}
