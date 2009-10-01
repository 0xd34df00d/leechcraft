/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#ifndef PLUGINS_SEEKTHRU_CORE_H
#define PLUGINS_SEEKTHRU_CORE_H
#include <QAbstractItemModel>
#include <QMap>
#include <interfaces/structures.h>
#include <interfaces/iinfo.h>
#include <interfaces/ifinder.h>
#include "description.h"
#include "searchhandler.h"

class IWebBrowser;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace SeekThru
		{
			class Core : public QAbstractItemModel
			{
				Q_OBJECT

				QMap<QString, QObject*> Providers_;
				QObjectList Downloaders_;
				QMap<int, QString> Jobs_;
				QList<Description> Descriptions_;
				QStringList Headers_;
				ICoreProxy_ptr Proxy_;

				static const QString OS_;

				Core ();
			public:
				enum Roles
				{
					RoleDescription = 200,
					RoleContact,
					RoleTags,
					RoleLongName,
					RoleDeveloper,
					RoleAttribution,
					RoleRight,
					RoleAdult,
					RoleLanguages
				};

				static Core& Instance ();

				void SetProxy (ICoreProxy_ptr);
				ICoreProxy_ptr GetProxy () const;

				virtual int columnCount (const QModelIndex& = QModelIndex ()) const;
				virtual QVariant data (const QModelIndex&, int = Qt::DisplayRole) const;
				virtual Qt::ItemFlags flags (const QModelIndex&) const;
				virtual QVariant headerData (int, Qt::Orientation, int = Qt::DisplayRole) const;
				virtual QModelIndex index (int, int, const QModelIndex& = QModelIndex()) const;
				virtual QModelIndex parent (const QModelIndex&) const;
				virtual int rowCount (const QModelIndex& = QModelIndex ()) const;

				void SetProvider (QObject*, const QString&);
				bool CouldHandle (const LeechCraft::DownloadEntity&) const;
				/** Fetches the searcher from the url.
				 *
				 * @param[in] url The url with the search description.
				 */
				void Add (const QUrl& url);
				void Remove (const QModelIndex&);
				void SetTags (const QModelIndex&, const QStringList&);
				QStringList GetCategories () const;
				IFindProxy_ptr GetProxy (const LeechCraft::Request&);
				IWebBrowser* GetWebBrowser () const;
				void HandleEntity (const QString&, const QString& = QString ());
			private:
				QStringList ComputeUniqueCategories () const;
				Description ParseData (const QString&, const QString&);
				void HandleProvider (QObject*);
				void ReadSettings ();
				void WriteSettings ();
			private slots:
				void handleJobFinished (int);
				void handleJobError (int);
			signals:
				void error (const QString&);
				void warning (const QString&);
				void delegateEntity (const LeechCraft::DownloadEntity&,
						int*, QObject**);
				void gotEntity (const LeechCraft::DownloadEntity&);
				void categoriesChanged (const QStringList&, const QStringList&);
			};
		};
	};
};

#endif

