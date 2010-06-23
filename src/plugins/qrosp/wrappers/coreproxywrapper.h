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

#ifndef PLUGINS_QROSP_WRAPPERS_COREPROXYWRAPPER_H
#define PLUGINS_QROSP_WRAPPERS_COREPROXYWRAPPER_H
#include <interfaces/iinfo.h>
#include <QMap>
#include <QIcon>
#include <QStringList>
#include <QModelIndex>

class QNetworkAccessManager;
class QTreeView;
class QTabWidget;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Qrosp
		{
			class CoreProxyWrapper : public QObject
			{
				Q_OBJECT

				ICoreProxy_ptr Proxy_;
			public:
				CoreProxyWrapper (ICoreProxy_ptr);
			public slots:
				QNetworkAccessManager* GetNetworkAccessManager () const;
				QObject* GetShortcutProxy () const;
				QModelIndex MapToSource (const QModelIndex&) const;
				//LeechCraft::Util::BaseSettingsManager* GetSettingsManager () const;
				QMap<int, QString> GetIconPath (const QString& name) const;
				QIcon GetIcon (const QString& on, const QString& off = QString ()) const;
				QMainWindow* GetMainWindow () const;
				QTabWidget* GetTabWidget () const;
				QObject* GetTagsManager () const;
				QStringList GetSearchCategories () const;
				int GetID ();
				void FreeID (int id);
				QObject* GetPluginsManager () const;
				QObject* GetSelf ();
			};
		};
	};
};

#endif
