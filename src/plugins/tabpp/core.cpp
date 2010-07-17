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

#include "core.h"
#include <QTabBar>
#include <QMainWindow>
#include <QApplication>
#include <QSortFilterProxyModel>
#include <QDynamicPropertyChangeEvent>
#include <plugininterface/treeitem.h>
#include <interfaces/imultitabs.h>

bool operator< (const QStringList& left, const QStringList& right)
{
	if (left.size () < right.size ())
		return true;

	// Left is empty, but not less than right => right is
	// empty as well, and stringlists are equal.
	if (left.isEmpty ())
		return false;

	// We should compare elements from the end.
	int delta = left.size () - right.size ();
	for (int i = right.size () - 1; i >= 0; --i)
		if (left.at (i + delta) < right.at (i))
			return true;

	return false;
}

namespace LeechCraft
{
	namespace Plugins
	{
		namespace TabPP
		{
			Core::Core ()
			: Bar_ (0)
			, TabWidget_ (0)
			, RootItem_ (new Util::TreeItem (QList<QVariant> () << QVariant ()))
			, Sorter_ (new QSortFilterProxyModel (this))
			, Current_ (-1)
			{
				Sorter_->setSourceModel (this);
				Sorter_->setDynamicSortFilter (true);
				Sorter_->sort (0);
			}

			Core& Core::Instance ()
			{
				static Core c;
				return c;
			}

			void Core::SetProxy (ICoreProxy_ptr proxy)
			{
				Proxy_ = proxy;

				Bar_ = proxy->GetMainWindow ()->
					findChild<QTabBar*> ("org_LeechCraft_MainWindow_CentralTabBar");
				if (!Bar_)
				{
					qWarning () << Q_FUNC_INFO
						<< "no tabbar found"
						<< proxy->GetMainWindow ()->findChildren<QTabBar*> ();
					return;
				}

				TabWidget_ = proxy->GetMainWindow ()->
					findChild<QTabWidget*> ("org_LeechCraft_MainWindow_CentralTabWidget");
				if (!TabWidget_ )
				{
					qWarning () << Q_FUNC_INFO
						<< "no tabwidget found"
						<< proxy->GetMainWindow ()->findChildren<QTabWidget*> ();
					return;
				}

				for (int i = 0; i < Bar_->count (); ++i)
					handleTabInserted (i);

				connect (Bar_,
						SIGNAL (tabWasInserted (int)),
						this,
						SLOT (handleTabInserted (int)));
				connect (Bar_,
						SIGNAL (tabWasRemoved (int)),
						this,
						SLOT (handleTabRemoved (int)));
				connect (Bar_,
						SIGNAL (currentChanged (int)),
						this,
						SLOT (handleCurrentChanged (int)));
				connect (Bar_,
						SIGNAL (tabMoved (int, int)),
						this,
						SLOT (handleTabsSwapped (int, int)));
			}

			ICoreProxy_ptr Core::GetProxy () const
			{
				return Proxy_;
			}

			QAbstractItemModel* Core::GetModel ()
			{
				return Sorter_;
			}

			void Core::HandleSelected (const QModelIndex& index)
			{
				QModelIndex ours = Sorter_->mapToSource (index);
				if (rowCount (ours))
					return;

				Util::TreeItem *item = static_cast<Util::TreeItem*> (ours.internalPointer ());
				if (!Child2Widget_.contains (item))
				{
					qWarning () << Q_FUNC_INFO
						<< "no widget for item"
						<< index
						<< item
						<< Child2Widget_;
					return;
				}
				QWidget *w = Child2Widget_ [item];
				TabWidget_->setCurrentWidget (w);
			}

			int Core::columnCount (const QModelIndex& index) const
			{
				if (index.isValid ())
					return static_cast<Util::TreeItem*> (index.internalPointer ())->ColumnCount ();
				else
					return RootItem_->ColumnCount ();
			}

			QVariant Core::data (const QModelIndex& index, int role) const
			{
				if (!index.isValid ())
					return QVariant ();

				Util::TreeItem *item = static_cast<Util::TreeItem*> (index.internalPointer ());
				switch (role)
				{
					case Qt::DisplayRole:
					case Qt::ToolTipRole:
						return item->Data (index.column ());
					case Qt::FontRole:
						if (item->Data (0, CRWidget).value<QWidget*> () != Pos2Widget_ [Current_])
							return QVariant ();
						else
						{
							QFont font = QApplication::font ();
							font.setBold (true);
							return font;
						}
					case Qt::DecorationRole:
						return index.column () ?
							QVariant () :
							item->Data (0, role);
					default:
						return QVariant ();
				}
			}

			QModelIndex Core::index (int row, int col, const QModelIndex& parent) const
			{
				if (!hasIndex (row, col, parent))
					return QModelIndex ();

				Util::TreeItem *parentItem;

				if (!parent.isValid ())
					parentItem = RootItem_;
				else
					parentItem = static_cast<Util::TreeItem*> (parent.internalPointer ());

				Util::TreeItem *childItem = parentItem->Child (row);
				if (childItem)
					return createIndex (row, col, childItem);
				else
					return QModelIndex ();
			}

			QModelIndex Core::parent (const QModelIndex& index) const
			{
				if (!index.isValid ())
					return QModelIndex ();

				Util::TreeItem *childItem = static_cast<Util::TreeItem*> (index.internalPointer ()),
						 *parentItem = childItem->Parent ();

				if (parentItem == RootItem_)
					return QModelIndex ();

				return createIndex (parentItem->Row (), 0, parentItem);
			}

			int Core::rowCount (const QModelIndex& parent) const
			{
				Util::TreeItem *parentItem;
				if (parent.column () > 0)
					return 0;

				if (!parent.isValid ())
					parentItem = RootItem_;
				else
					parentItem = static_cast<Util::TreeItem*> (parent.internalPointer ());

				return parentItem->ChildCount ();
			}

			bool Core::eventFilter (QObject *obj, QEvent *event)
			{
				if (event->type () == QEvent::DynamicPropertyChange)
				{
					QWidget *widget = qobject_cast<QWidget*> (obj);
					if (widget &&
							static_cast<QDynamicPropertyChangeEvent*> (event)->
								propertyName () == "WidgetLogicalPath")
							HandleLogicalPathChanged (widget);
					else
						qWarning () << Q_FUNC_INFO
							<< obj
							<< "not a QWidget*";
					return false;
				}
				else
					return QAbstractItemModel::eventFilter (obj, event);
			}

			Util::TreeItem* Core::Find (const QString& part,
					Util::TreeItem *parent, QWidget *widget) const
			{
				Util::TreeItem *result = 0;
				for (int i = 0; i < parent->ChildCount (); ++i)
				{
					Util::TreeItem *child = parent->Child (i);
					QWidget *chwd = child->Data (0, CRWidget).value<QWidget*> ();
					if (child->Data (0, CRRawPath).toString () == part &&
							(!chwd ||
							 chwd == widget))
					{
						result = child;
						break;
					}
				}
				return result;
			}

			void Core::HandleLogicalPathChanged (QWidget *widget)
			{
				int idx = TabWidget_->indexOf (widget);
				if (idx < 0)
					return;

				CleanUpRemovedLogicalPath (widget);
				QStringList parts = widget->
						property ("WidgetLogicalPath").toStringList ();
				if (parts.isEmpty ())
				{
					QString title = Bar_->tabText (idx);
					if (title.isEmpty ())
						title = tr ("unknown");
					parts << title;
				}

				Util::TreeItem *previous = RootItem_;
				QModelIndex previousIndex;
				Q_FOREACH (QString part, parts)
				{
					Util::TreeItem *child = Find (part, previous, widget);
					if (!child)
					{
						QList<QVariant> showData;
						showData << part;
						child = new Util::TreeItem (showData, previous);
						child->ModifyData (0, part, CRRawPath);

						int pos = rowCount (previousIndex);
						beginInsertRows (previousIndex, pos, pos);
						previous->AppendChild (child);
						endInsertRows ();
					}
					previousIndex = index (previous->ChildPosition (child),
							0, previousIndex);
					previous = child;
				}
				previous->ModifyData (0,
						QVariant::fromValue<QWidget*> (widget), CRWidget);
				Path2Child_ [parts] = previous;
				Child2Path_ [previous] = parts;
				Widget2Child_ [widget] = previous;
				Child2Widget_ [previous] = widget;
			}

			void Core::CleanUpRemovedLogicalPath (QWidget *w)
			{
				if (!Widget2Child_.contains (w))
					return;

				Util::TreeItem *child = Widget2Child_ [w];

				QList<int> posHier;
				while (child->Parent ())
				{
					int pos = child->Parent ()->ChildPosition (child);
					posHier.prepend (pos);
					child = child->Parent ();
				}

				child = Widget2Child_ [w];

				posHier.takeLast ();

				QList<QModelIndex> indexesHier;
				QModelIndex prevIndex;
				indexesHier << prevIndex;
				Q_FOREACH (int pos, posHier)
				{
					prevIndex = index (pos, 0, prevIndex);
					indexesHier << prevIndex;
				}

				while (!child->ChildCount ())
				{
					Util::TreeItem *parent = child->Parent ();
					if (!parent)
						break;
					int pos = parent->ChildPosition (child);
					beginRemoveRows (indexesHier.takeLast (), pos, pos);
					parent->RemoveChild (pos);
					endRemoveRows ();
					child = parent;
				}

				Path2Child_.remove (Child2Path_ [child]);
				Child2Path_.remove (child);
				Widget2Child_.remove (w);
				Child2Widget_.remove (child);
			}

			QModelIndex Core::GetIndexForItem (const Util::TreeItem *item) const
			{
				if (!item)
					return QModelIndex ();

				QList<int> posHier;
				const Util::TreeItem *child = item;
				while (child->Parent ())
				{
					int pos = child->Parent ()->ChildPosition (child);
					posHier.prepend (pos);;
					child = child->Parent ();
				}

				QModelIndex prevIndex;
				Q_FOREACH (int pos, posHier)
					prevIndex = index (pos, 0, prevIndex);

				return prevIndex;
			}

			void Core::handleTabInserted (int idx)
			{
				QWidget *widget = TabWidget_->widget (idx);

				bool initConnections = !Widget2Pos_.contains (widget);

				Pos2Widget_ [idx] = widget;
				Widget2Pos_ [widget] = idx;
				widget->installEventFilter (this);

				HandleLogicalPathChanged (widget);

				if (initConnections)
				{
					QObject *obj = 0;
					IMultiTabsWidget *imtw = qobject_cast<IMultiTabsWidget*> (widget);
					if (imtw)
						obj = imtw->ParentMultiTabs ();
					else
						obj = widget;
					if (obj->metaObject ()->indexOfSignal (QMetaObject::normalizedSignature (
									"changeTabIcon (QWidget*, const QIcon&)"
									).constData ()) != -1)
						connect (obj,
								SIGNAL (changeTabIcon (QWidget*, const QIcon&)),
								this,
								SLOT (handleChangeTabIcon (QWidget*, const QIcon&)));
					handleChangeTabIcon (widget, TabWidget_->tabIcon (idx));
				}
			}

			void Core::handleTabRemoved (int idx)
			{
				QWidget *w = Pos2Widget_ [idx];

				CleanUpRemovedLogicalPath (w);

				Pos2Widget_.remove (idx);
				Widget2Pos_.remove (w);

				while (true)
				{
					if (!Pos2Widget_.contains (idx + 1))
						break;
					Pos2Widget_ [idx] = Pos2Widget_ [idx + 1];
					QWidget *w = Pos2Widget_ [idx];

					Widget2Pos_ [w] = idx;
					++idx;
				}
				Pos2Widget_.remove (idx);
			}

			void Core::handleTabsSwapped (int a, int b)
			{
				std::swap (Pos2Widget_ [a], Pos2Widget_ [b]);
				QWidget *wa = Pos2Widget_ [a];
				QWidget *wb = Pos2Widget_ [b];
				Widget2Pos_ [wa] = b;
				Widget2Pos_ [wb] = a;
			}

			void Core::handleCurrentChanged (int idx)
			{
				QWidget *prev = 0;
				if (Pos2Widget_.contains (Current_))
					prev = Pos2Widget_ [Current_];

				Current_ = idx;
				QWidget *cw = Pos2Widget_ [Current_];
				if (prev)
				{
					QModelIndex pi = GetIndexForItem (Widget2Child_ [prev]);
					emit dataChanged (pi,
							pi.sibling (pi.row (), columnCount (pi.parent ()) - 1));
				}

				if (cw)
				{
					QModelIndex ci = GetIndexForItem (Widget2Child_ [cw]);
					emit dataChanged (ci,
							ci.sibling (ci.row (), columnCount (ci.parent ()) - 1));
				}
			}

			void Core::handleChangeTabIcon (QWidget *widget, const QIcon& icon)
			{
				if (!Widget2Child_.contains (widget))
					return;

				Util::TreeItem *item = Widget2Child_ [widget];
				item->ModifyData (0, icon, Qt::DecorationRole);
				QModelIndex index = GetIndexForItem (item);
				emit dataChanged (index, index);
			}
		};
	};
};

