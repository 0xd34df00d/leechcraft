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
#include <interfaces/iinfo.h>

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
				history_items_t Items_;
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
			public slots:
				void addItem (QString, QString, QDateTime);
				QList<QMap<QString, QVariant> > getItemsMap () const;
			private:
				void Add (const HistoryItem&);
			private slots:
				void loadData ();
				void handleItemAdded (const HistoryItem&);
			signals:
				// Hook support signals
				/** @brief Called when an entry is going to be added to
				 * history.
				 *
				 * If the proxy is cancelled, no addition takes place
				 * at all. If it is not, the return value from the proxy
				 * is considered as a list of QVariants. First element
				 * (if any) would be converted to string and replace
				 * title, second element (if any) would be converted to
				 * string and replace url, third element (if any) would
				 * be converted to QDateTime and replace the date.
				 */
				void hookAddingToHistory (LeechCraft::IHookProxy_ptr proxy,
						QString title, QString url, QDateTime date);
			};
		};
	};
};

Q_DECLARE_METATYPE (LeechCraft::Plugins::Poshuku::HistoryItem);


#endif

