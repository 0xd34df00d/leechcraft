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

#ifndef PLUGINS_POSHUKU_URLCOMPLETIONMODEL_H
#define PLUGINS_POSHUKU_URLCOMPLETIONMODEL_H
#include <QAbstractItemModel>
#include <interfaces/iinfo.h>
#include "historymodel.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			class URLCompletionModel : public QAbstractItemModel
			{
				Q_OBJECT

				mutable bool Valid_;
				mutable history_items_t Items_;
				QString Base_;
			public:
				enum
				{
					RoleURL = 45
				};
				URLCompletionModel (QObject* = 0);
				virtual ~URLCompletionModel ();

				virtual int columnCount (const QModelIndex& = QModelIndex ()) const;
				virtual QVariant data (const QModelIndex&, int = Qt::DisplayRole) const;
				virtual Qt::ItemFlags flags (const QModelIndex&) const;
				virtual QVariant headerData (int, Qt::Orientation,
						int = Qt::DisplayRole) const;
				virtual QModelIndex index (int, int,
						const QModelIndex& = QModelIndex()) const;
				virtual QModelIndex parent (const QModelIndex&) const;
				virtual int rowCount (const QModelIndex& = QModelIndex ()) const;
			public slots:
				void setBase (const QString&);
				void handleItemAdded (const HistoryItem&);
				void addItem (const QString& title, const QString& url);
			signals:
				void baseUpdated (QObject*);

				// Plugin API
				void hookURLCompletionNewStringRequested (LeechCraft::IHookProxy_ptr proxy,
						QObject *model,
						const QString& string,
						int historyItems);
			private:
				void Populate ();
			};
		};
	};
};

#endif

