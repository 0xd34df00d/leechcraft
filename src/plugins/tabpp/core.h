/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010  Georg Rudoy
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

#ifndef PLUGINS_TABPP_CORE_H
#define PLUGINS_TABPP_CORE_H
#include <QAbstractItemModel>
#include "interfaces/iinfo.h"

class QTabBar;
class QSortFilterProxyModel;

namespace LeechCraft
{
	namespace Util
	{
		class TreeItem;
	};

	namespace Plugins
	{
		namespace TabPP
		{
			class Core : public QAbstractItemModel
			{
				Q_OBJECT

				ICoreProxy_ptr Proxy_;
				QTabBar *Bar_;
				QTabWidget *TabWidget_;
				Util::TreeItem *RootItem_;
				QSortFilterProxyModel *Sorter_;

				QMap<QString, Util::TreeItem*> Path2Child_;
				QMap<Util::TreeItem*, QString> Child2Path_;
				QMap<QWidget*, Util::TreeItem*> Widget2Child_;
				QMap<Util::TreeItem*, QWidget*> Child2Widget_;
				QMap<int, QWidget*> Pos2Widget_;
				QMap<QWidget*, int> Widget2Pos_;
				int Current_;

				Core ();

				enum CustomRoles
				{
					CRRawPath,
					CRWidget
				};
			public:
				static Core& Instance ();
				void SetProxy (ICoreProxy_ptr);
				ICoreProxy_ptr GetProxy () const;
				QAbstractItemModel* GetModel ();
				void HandleSelected (const QModelIndex&);

				virtual int columnCount (const QModelIndex& = QModelIndex ()) const;
				virtual QVariant data (const QModelIndex&, int = Qt::DisplayRole) const;
				virtual QModelIndex index (int, int, const QModelIndex& = QModelIndex()) const;
				virtual QModelIndex parent (const QModelIndex&) const;
				virtual int rowCount (const QModelIndex& = QModelIndex ()) const;
			protected:
				bool eventFilter (QObject*, QEvent*);
			private:
				Util::TreeItem* Find (const QString&, Util::TreeItem*, QWidget*) const;
				void HandleLogicalPathChanged (QWidget*);
				void CleanUpRemovedLogicalPath (QWidget*);
				QModelIndex GetIndexForItem (const Util::TreeItem*) const;
			private slots:
				void handleTabInserted (int);
				void handleTabRemoved (int);
				void handleTabsSwapped (int, int);
				void handleCurrentChanged (int);
			};
		};
	};
};

#endif

