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

#include "treeviewreemitter.h"
#include "summarywidget.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Summary
		{
			TreeViewReemitter::TreeViewReemitter (QObject *parent)
			: QObject (parent)
			{
			}

			void TreeViewReemitter::Connect (SummaryWidget *tc)
			{
				QTreeView *view = tc->GetUi ().PluginsTasksTree_;
#define C(x) \
				connect (view, \
						SIGNAL (x (const QModelIndex&)), \
						this, \
						SLOT (handle_##x (const QModelIndex&)));
				C (activated);
				C (clicked);
				C (doubleClicked);
				C (entered);
				C (pressed);
#undef C
				connect (view,
						SIGNAL (viewportEntered ()),
						this,
						SLOT (handleViewportEntered ()));
			}

			void TreeViewReemitter::ConnectModelSpecific (SummaryWidget *tc)
			{
				QItemSelectionModel *sel = tc->GetUi ().PluginsTasksTree_->selectionModel ();
#define C(x) \
				connect (sel, \
						SIGNAL (x (const QModelIndex&, const QModelIndex&)), \
						this, \
						SLOT (handle_##x (const QModelIndex&, const QModelIndex&)));
				C (currentChanged);
				C (currentColumnChanged);
				C (currentRowChanged);
#undef C
				connect (sel,
						SIGNAL (selectionChanged (const QItemSelection&, const QItemSelection&)),
						this,
						SLOT (handleSelectionChanged (const QItemSelection&, const QItemSelection&)));
			}

#define D(x) \
			void TreeViewReemitter::handle_##x (const QModelIndex& index) \
			{ \
				emit x (index, qobject_cast<QTreeView*> (sender ())); \
			}
			D (activated);
			D (clicked);
			D (doubleClicked);
			D (entered);
			D (pressed);
#undef D

			void TreeViewReemitter::handleViewportEntered ()
			{
				emit viewportEntered (qobject_cast<QTreeView*> (sender ()));
			}

#define D(x) \
			void TreeViewReemitter::handle_##x (const QModelIndex& current, \
					const QModelIndex& previous) \
			{ \
				emit x (current, previous, qobject_cast<QTreeView*> (sender ())); \
			}
			D (currentChanged);
			D (currentColumnChanged);
			D (currentRowChanged);
#undef D

			void TreeViewReemitter::handleSelectionChanged (const QItemSelection& selected,
					const QItemSelection& deselected)
			{
				emit selectionChanged (selected, deselected, qobject_cast<QTreeView*> (sender ()));
			}
		};
	};
};

