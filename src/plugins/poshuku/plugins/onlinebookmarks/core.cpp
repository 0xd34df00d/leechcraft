/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010  Oleg Linkin
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
#include <QtDebug>
#include <QCoreApplication>
#include <QNetworkAccessManager>
#include <QStandardItemModel>
#include <QSettings>
#include <plugininterface/util.h>
#include <interfaces/iproxyobject.h>
#include "syncbookmarks.h"

namespace LeechCraft
{
namespace Plugins
{
namespace Poshuku
{
namespace Plugins
{
namespace OnlineBookmarks
{
	Core::Core ()
	{
		 Init ();
	}

	Core& Core::Instance ()
	{
		static Core c;
		return c;
	}

	void Core::SendEntity (const LeechCraft::Entity& e)
	{
		emit gotEntity (e);
	}

	void Core::Init ()
	{
		Model_ = new QStandardItemModel;
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Poshuku_OnlineBookmarks");
		settings.beginGroup ("Account");
		
		Q_FOREACH (const QString& item, settings.childKeys ())
		{
			QList<QStandardItem*> itemList;
			
			Q_FOREACH (const QString& login, settings.value (item).toStringList ())
			{
				QStandardItem *loginItem = new QStandardItem (login);
				itemList << loginItem;
			}
			QStandardItem *service = new QStandardItem (item);
			Model_->appendRow (service);
			service->appendRows (itemList);
		}
		settings.endGroup ();
		
		BookmarksSyncManager_ = new SyncBookmarks (this);
	}

	QStandardItemModel* Core::GetAccountModel () const
	{
		return Model_;
	}
	
	SyncBookmarks* Core::GetBookmarksSyncManager () const
	{
		return BookmarksSyncManager_;
	}
	
	void Core::SetActiveBookmarksServices (QList<AbstractBookmarksService*> list)
	{
		ActiveBookmarksServices_ = list;
	}
	
	QList<AbstractBookmarksService*> Core::GetActiveBookmarksServices () const
	{
		return ActiveBookmarksServices_;
	}

	void Core::SetPassword (const QString& password, const QString& account, const QString& service)
	{
		QList<QVariant> keys;
		keys << "org.LeechCraft.Poshuku.OnlineBookmarks." +
				service + "/" + account;

		QList<QVariant> passwordVar;
		passwordVar << password;
		QList<QVariant> values;
		values << QVariant (passwordVar);

		Entity e = Util::MakeEntity (keys,
				QString (),
				Internal,
				"x-leechcraft/data-persistent-save");
		e.Additional_ ["Values"] = values;
		e.Additional_ ["Overwrite"] = true;

		Core::Instance ().SendEntity (e);
	}

	QString Core::GetPassword (const QString& account, const QString& service) const
	{
		QList<QVariant> keys;
		keys << "org.LeechCraft.Poshuku.OnlineBookmarks." + service + "/" + account;
		const QVariantList& result =
				Util::GetPersistentData (keys, &Core::Instance ());
		if (result.size () != 1)
		{
			qWarning () << Q_FUNC_INFO
					<< "incorrect result size"
					<< result;
			return QString ();
		}

		const QVariantList& strVarList = result.at (0).toList ();
		if (strVarList.isEmpty () ||
				!strVarList.at (0).canConvert<QString> ())
		{
			qWarning () << Q_FUNC_INFO
					<< "invalid string variant list"
					<< strVarList;
			return QString ();
		}

		return strVarList.at (0).toString ();
	}
	
	QNetworkAccessManager* Core::GetNetworkAccessManager () const
	{
		return Proxy_->GetNetworkAccessManager ();
	}

	void Core::SetProxy (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;
	}
	
	ICoreProxy_ptr Core::GetProxy () const
	{
		return Proxy_;
	}
	
	void Core::SetPluginProxy (QObject *proxy)
	{
		PluginProxy_ = proxy;
	}

	QObject* Core::GetBookmarksModel () const
	{
		IProxyObject *obj = qobject_cast<IProxyObject*> (PluginProxy_);
		if (!obj)
		{	
			qWarning () << Q_FUNC_INFO
					<< "obj is not an IProxyObject"
					<< PluginProxy_;
			return 0;
		}
		
		return obj->GetFavoritesModel ();
	}
	
	QDir Core::GetBookmarksDir () const
	{
		return BookmarksDir_;
	}

	void Core::SetBookamrksDir (const QDir& path)
	{
		BookmarksDir_ = path;
	}
}
}
}
}
}

