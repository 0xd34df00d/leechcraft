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

#ifndef PLUGINS_PYLC_COREPROXYWRAPPER_H
#define PLUGINS_PYLC_COREPROXYWRAPPER_H
#include <QObject>
#include <QModelIndex>
#include <QIcon>
#include <QMainWindow>
#include <QTabWidget>
#include <interfaces/iinfo.h>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace PyLC
		{
			class CoreProxyWrapper : public QObject
			{
				Q_OBJECT

				ICoreProxy_ptr W_;
			public:
				CoreProxyWrapper (ICoreProxy_ptr);
			public slots:
				QNetworkAccessManager* GetNetworkAccessManager () const;
				/*
				const IShortcutProxy* GetShortcutProxy () const;
				*/
				QTreeView* GetCurrentView () const;
				QModelIndex MapToSource (const QModelIndex&) const;
				/*
				Util::BaseSettingsManager* GetSettingsManager () const;
				*/
				QIcon GetIcon (const QString& on, const QString& off = QString ()) const;
				QMainWindow* GetMainWindow () const;
				QTabWidget* GetTabWidget () const;
				/*
				ITagsManager* GetTagsManager () const;
				*/
				QStringList GetSearchCategories () const;
				int GetID ();
				void FreeID (int);
				QObject* GetTreeViewReemitter () const;
				/*
				IPluginsManager* GetPluginsManager () const;
				*/
				QObject* GetSelf ();
			};
		};
	};
};

#endif

