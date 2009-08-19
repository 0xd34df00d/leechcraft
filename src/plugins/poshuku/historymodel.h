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

#ifndef PLUGINS_POSHUKU_HISTORYMODEL_H
#define PLUGINS_POSHUKU_HISTORYMODEL_H
#include <deque>
#include <vector>
#include <QAbstractItemModel>
#include <QStringList>
#include <QDateTime>

class QTimer;
class QAction;

namespace LeechCraft
{
	namespace Util
	{
		class TreeItem;
	};

	namespace Plugins
	{
		namespace Poshuku
		{
			struct HistoryItem
			{
				QString Title_;
				QDateTime DateTime_;
				QString URL_;
			};

			typedef std::vector<HistoryItem> history_items_t;

			class HistoryModel : public QAbstractItemModel
			{
				Q_OBJECT

				QTimer *GarbageTimer_;
				Util::TreeItem *RootItem_;
				QAction *FolderIconProxy_;
			public:
				enum Columns
				{
					ColumnTitle
					, ColumnURL
					, ColumnDate
				};

				HistoryModel (QObject* = 0);
				virtual ~HistoryModel ();

				int columnCount (const QModelIndex& = QModelIndex ()) const;
				QVariant data (const QModelIndex&, int = Qt::DisplayRole) const;
				Qt::ItemFlags flags (const QModelIndex&) const;
				QVariant headerData (int, Qt::Orientation, int = Qt::DisplayRole) const;
				QModelIndex index (int, int, const QModelIndex& = QModelIndex()) const;
				QModelIndex parent (const QModelIndex&) const;
				int rowCount (const QModelIndex& = QModelIndex ()) const;

				void AddItem (const QString&, const QString&, const QDateTime&);
			private:
				void Add (const HistoryItem&);
			private slots:
				void loadData ();
				void handleItemAdded (const HistoryItem&);
			};
		};
	};
};

Q_DECLARE_METATYPE (LeechCraft::Plugins::Poshuku::HistoryItem);


#endif

