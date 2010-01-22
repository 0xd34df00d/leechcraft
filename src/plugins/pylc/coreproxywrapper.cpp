/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2009  Georg Rudoy
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
#include "shortcutproxywrapper.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace PyLC
		{
			CoreProxyWrapper::CoreProxyWrapper (ICoreProxy_ptr wrapped)
			: W_ (wrapped)
			{
			}

			QNetworkAccessManager* CoreProxyWrapper::GetNetworkAccessManager () const
			{
				return W_->GetNetworkAccessManager ();
			}

			ShortcutProxyWrapper* CoreProxyWrapper::GetShortcutProxy () const
			{
				return new ShortcutProxyWrapper (W_->GetShortcutProxy ());
			}

			QTreeView* CoreProxyWrapper::GetCurrentView () const
			{
				return W_->GetCurrentView ();
			}

			QModelIndex CoreProxyWrapper::MapToSource (const QModelIndex& i) const
			{
				return W_->MapToSource (i);
			}

			QIcon CoreProxyWrapper::GetIcon (const QString& on, const QString& off) const
			{
				return W_->GetIcon (on, off);
			}

			QMainWindow* CoreProxyWrapper::GetMainWindow () const
			{
				return W_->GetMainWindow ();
			}

			QTabWidget* CoreProxyWrapper::GetTabWidget () const
			{
				return W_->GetTabWidget ();
			}

			QStringList CoreProxyWrapper::GetSearchCategories () const
			{
				return W_->GetSearchCategories ();
			}

			int CoreProxyWrapper::GetID ()
			{
				return W_->GetID ();
			}

			void CoreProxyWrapper::FreeID (int id)
			{
				W_->FreeID (id);
			}

			QObject* CoreProxyWrapper::GetTreeViewReemitter () const
			{
				return W_->GetTreeViewReemitter ();
			}

			QObject* CoreProxyWrapper::GetSelf ()
			{
				return W_->GetSelf ();
			}
		};
	};
};

