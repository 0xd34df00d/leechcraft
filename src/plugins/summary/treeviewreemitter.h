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

#ifndef PLUGINS_SUMMARY_TREEVIEWREEMITTER_H
#define PLUGINS_SUMMARY_TREEVIEWREEMITTER_H
#include <QObject>

class QModelIndex;
class QItemSelection;
class QTreeView;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Summary
		{
			class SummaryWidget;

			class TreeViewReemitter : public QObject
			{
				Q_OBJECT
			public:
				TreeViewReemitter (QObject* = 0);

				void Connect (SummaryWidget*);
				void ConnectModelSpecific (SummaryWidget*);
			public slots:
				void handle_activated (const QModelIndex&);
				void handle_clicked (const QModelIndex&);
				void handle_doubleClicked (const QModelIndex&);
				void handle_entered (const QModelIndex&);
				void handle_pressed (const QModelIndex&);
				void handleViewportEntered ();

				void handle_currentChanged (const QModelIndex&, const QModelIndex&);
				void handle_currentColumnChanged (const QModelIndex&, const QModelIndex&);
				void handle_currentRowChanged (const QModelIndex&, const QModelIndex&);
				void handleSelectionChanged (const QItemSelection&, const QItemSelection&);
			signals:
				void activated (const QModelIndex&, QTreeView*);
				void clicked (const QModelIndex&, QTreeView*);
				void doubleClicked (const QModelIndex&, QTreeView*);
				void entered (const QModelIndex&, QTreeView*);
				void pressed (const QModelIndex&, QTreeView*);
				void viewportEntered (QTreeView*);

				void currentChanged (const QModelIndex&, const QModelIndex&, QTreeView*);
				void currentColumnChanged (const QModelIndex&, const QModelIndex&, QTreeView*);
				void currentRowChanged (const QModelIndex&, const QModelIndex&, QTreeView*);
				void selectionChanged (const QItemSelection&, const QItemSelection&, QTreeView*);
			};
		};
	};
};

#endif

