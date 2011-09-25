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
#include <interfaces/iplugin2.h>
#include <interfaces/iserviceplugin.h>
#include "accountssettings.h"
#include "pluginmanager.h"
#include "interfaces/iaccount.h"

namespace LeechCraft
{
namespace Poshuku
{
namespace OnlineBookmarks
{
	Core::Core ()
	: PluginManager_ (new PluginManager)
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

	void Core::SetPluginProxy (QObject* proxy)
	{
		PluginProxy_ = proxy;
	}

	AccountsSettings* Core::GetAccountsSettingsWidget () const
	{
		return AccountsSettings_;
	}

	QSet<QByteArray> Core::GetExpectedPluginClasses () const
	{
		QSet<QByteArray> classes;
		classes << "org.LeechCraft.Plugins.Poshuku.Plugins.OnlineBookmarks.IServicePlugin";
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

		PluginManager_->AddPlugin (plugin);

		QSet<QByteArray> classes = plugin2->GetPluginClasses ();
		if (classes.contains ("org.LeechCraft.Plugins.Poshuku.Plugins.OnlineBookmarks.IServicePlugin"))
		{
			IServicePlugin *service = qobject_cast<IServicePlugin*> (plugin);
			if (!service)
			{
				qWarning () << Q_FUNC_INFO
						<< "plugin"
						<< plugin
						<< "tells it implements the IServicePlugin but cast failed";
				return;
			}
			AddServicePlugin (service->GetBookmarksService ());
		}
	}

	void Core::AddServicePlugin (QObject* plugin)
	{
		IBookmarksService *ibs = qobject_cast<IBookmarksService*> (plugin);
		if (!plugin)
		{
			qWarning () << Q_FUNC_INFO
					<< plugin
					<< "is not an IBookmarksService";
			return;
		}

		ServicesPlugins_ << plugin;
	}

	QObjectList Core::GetServicePlugins () const
	{
		return ServicesPlugins_;
	}

	void Core::AddActiveAccount (QObject* obj)
	{
		if (!ActiveAccounts_.contains (obj))
			ActiveAccounts_ << obj;
	}

	void Core::SetActiveAccounts (QObjectList list)
	{
		ActiveAccounts_.clear ();
		ActiveAccounts_.append (list);
	}

	QObjectList Core::GetActiveAccounts () const
	{
		return ActiveAccounts_;
	}

	void Core::UploadBookmark(const QString& title,
			const QString& url, const QStringList& tags)
	{
		Q_FOREACH (QObject *accObj, ActiveAccounts_)
		{
			IAccount *account = qobject_cast<IAccount*> (accObj);
			IBookmarksService *ibs = qobject_cast<IBookmarksService*> (account->GetParentService ());
			QVariantList list;
			QVariantMap map;
			map ["Title"] = title;
			map ["Url"] = url;
			map ["Tags"] = tags;
			list << map;
			ibs->UploadBookmarks (account, list);
		}
	}

	void Core::syncBookmarks ()
	{

	}

	void Core::uploadBookmarks ()
	{

	}

	void Core::downloadBookmarks ()
	{

	}

	void Core::downloadAllBookmarks ()
	{

	}

}
}
}

