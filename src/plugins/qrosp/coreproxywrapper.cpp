/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2010  Georg Rudoy
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

#include "coreproxywrapper.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Qrosp
		{
			CoreProxyWrapper::CoreProxyWrapper (ICoreProxy_ptr proxy)
			: Proxy_ (proxy)
			{
			}

			QNetworkAccessManager* CoreProxyWrapper::GetNetworkAccessManager () const
			{
				return Proxy_->GetNetworkAccessManager ();
			}

			QTreeView* CoreProxyWrapper::GetCurrentView () const
			{
				return Proxy_->GetCurrentView ();
			}

			QModelIndex CoreProxyWrapper::MapToSource (const QModelIndex& index) const
			{
				return Proxy_->MapToSource (index);
			}

			QMap<int, QString> CoreProxyWrapper::GetIconPath (const QString& name) const
			{
				return Proxy_->GetIconPath (name);
			}

			QIcon CoreProxyWrapper::GetIcon (const QString& on, const QString& off) const
			{
				return Proxy_->GetIcon (on, off);
			}

			QMainWindow* CoreProxyWrapper::GetMainWindow () const
			{
				return Proxy_->GetMainWindow ();
			}

			QTabWidget* CoreProxyWrapper::GetTabWidget () const
			{
				return Proxy_->GetTabWidget ();
			}

			QStringList CoreProxyWrapper::GetSearchCategories () const
			{
				return Proxy_->GetSearchCategories ();
			}

			int CoreProxyWrapper::GetID ()
			{
				return Proxy_->GetID ();
			}

			void CoreProxyWrapper::FreeID (int id)
			{
				Proxy_->FreeID (id);
			}

			QObject* CoreProxyWrapper::GetTreeViewReemitter () const
			{
				return Proxy_->GetTreeViewReemitter ();
			}

			QObject* CoreProxyWrapper::GetSelf ()
			{
				return Proxy_->GetSelf ();
			}
		};
	};
};
