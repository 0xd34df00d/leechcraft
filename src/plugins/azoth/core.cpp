/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010  Georg Rudoy
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

#include "core.h"
#include <interfaces/iplugin2.h>
#include "interfaces/iprotocolplugin.h"
#include "azothserverconnection.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Azoth
		{
			Core::Core ()
			{
				ServerConnection_ = new AzothServerConnection ();
				connect (ServerConnection_,
						SIGNAL (gotEntity (const LeechCraft::DownloadEntity&)),
						this,
						SIGNAL (gotEntity (const LeechCraft::DownloadEntity&)));
			}

			Core& Core::Instance ()
			{
				static Core c;
				return c;
			}

			void Core::SetProxy (ICoreProxy_ptr proxy)
			{
				Proxy_ = proxy;
			}

			ICoreProxy_ptr Core::GetProxy () const
			{
				return Proxy_;
			}

			void Core::Init ()
			{
				ServerConnection_->Establish ();
			}

			void Core::Release ()
			{
				ServerConnection_->Release ();
			}

			QSet<QByteArray> Core::GetExpectedPluginClasses () const
			{
				QSet<QByteArray> classes;
				classes << "org.LeechCraft.Plugins.Azoth.Plugins.IProtocolPlugin";
				return classes;
			}

			void Core::AddPlugin (QObject *plugin)
			{
				IPlugin2 *plugin2 = qobject_cast<IPlugin2*> (plugin);
				if (!plugin2)
				{
					qWarning () << Q_FUNC_INFO
						<< plugin
						<< "isn't a IPlugin2";
					return;
				}

				QSet<QByteArray> classes = plugin2->GetPluginClasses ();
				if (classes.contains ("org.LeechCraft.Plugins.Azoth.Plugins.IProtocolPlugin"))
					AddProtocolPlugin (plugin);
			}

			const QObjectList& Core::GetProtocolPlugins () const
			{
				return ProtocolPlugins_;
			}

			void Core::AddProtocolPlugin (QObject *plugin)
			{
				Plugins::IProtocolPlugin *ipp =
					qobject_cast<Plugins::IProtocolPlugin*> (plugin);
				if (!ipp)
					qWarning () << Q_FUNC_INFO
						<< "plugin"
						<< plugin
						<< "tells it implements the IProtocolPlugin but cast failed";
				else
				{
					qDebug () << Q_FUNC_INFO
						<< "got protocol plugin"
						<< plugin;
					ProtocolPlugins_ << plugin;
				}
			}
		};
	};
};

