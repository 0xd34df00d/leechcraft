/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
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
#include <QStandardItemModel>
#include "accountssettings.h"
#include "pluginmanager.h"

namespace LeechCraft
{
namespace Poshuku
{
namespace OnlineBookmarks
{
	Core::Core ()
	: ActiveServicesModel_ (new QStandardItemModel)
	, PluginManager_ (new PluginManager)
	, AccountsSettings_ (new AccountsSettings)
	{
	}

	Core& Core::Instance ()
	{
		static Core c;
		return c;
	}

	void Core::SetProxy (ICoreProxy_ptr proxy)
	{
		CoreProxy_ = proxy;
	}

	ICoreProxy_ptr Core::GetProxy () const
	{
		return CoreProxy_;
	}

	QAbstractItemModel* Core::GetActiveServicesModel () const
	{
		return ActiveServicesModel_;
	}

	AccountsSettings* Core::GetAccountsSettingsWidget () const
	{
		return AccountsSettings_;
	}

	QSet<QByteArray> Core::GetExpectedPluginClasses () const
	{
		QSet<QByteArray> classes;
		classes << "org.LeechCraft.Plugins.Poshuku.Plugins.OnlineBookmarks.IGeneralPlugin";
		return classes;
	}

	void Core::AddPlugin (QObject *plugin)
	{
		PluginManager_->AddPlugin (plugin);
		ServicesPlugins_ << plugin;
	}

	QObjectList Core::GetPlugins () const
	{
		return ServicesPlugins_;
	}

	QList<IBookmarksService*> Core::GetBookmarksServices () const
	{
		QList<IBookmarksService*> result;
		Q_FOREACH (QObject *plugin, ServicesPlugins_)
		{
			IBookmarksService *bs = qobject_cast<IBookmarksService*> (plugin);
			if (!bs)
				continue;
			result << bs;
		}
		return result;
	}

}
}
}

