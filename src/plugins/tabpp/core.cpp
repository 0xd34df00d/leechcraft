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
#include <plugininterface/treeitem.h>

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
			{
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
			}

			ICoreProxy_ptr Core::GetProxy () const
			{
				return Proxy_;
			}

			QAbstractItemModel* Core::GetModel ()
			{
				return this;
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
			
				switch (role)
				{
					case Qt::DisplayRole:
						return static_cast<Util::TreeItem*> (index.internalPointer ())->
							Data (index.column ());
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

			Util::TreeItem* Core::Find (const QString& part,
					Util::TreeItem *parent, QWidget *widget)
			{
				Util::TreeItem *result = 0;
				for (int i = 0; i < parent->ChildCount (); ++i)
				{
					Util::TreeItem *child = parent->Child (i);
					if (child->Data (0, CRRawPath).toString () == part &&
							(!child->Data (0, CRWidget).value<QWidget*> () ||
							 child->Data (0, CRWidget).value<QWidget*> () == widget))
					{
						result = child;
						break;
					}
				}
				return result;
			}

			void Core::handleTabInserted (int idx)
			{
				QWidget *widget = TabWidget_->widget (idx);
				QString path = widget->property ("WidgetLogicalPath").toString ();
				QString removed = path;
				if (removed.remove ('/').isEmpty ())
				{
					QString title = Bar_->tabText (idx);
					if (title.isEmpty ())
						title = tr ("unknown");
					path = QString ("/%1").arg (title);
				}

				qDebug () << Q_FUNC_INFO << idx << path;

				QStringList parts = path.split ('/', QString::SkipEmptyParts);
				Util::TreeItem *previous = RootItem_;
				QModelIndex previousIndex;
				Q_FOREACH (QString part, parts)
				{
					Util::TreeItem *child = Find (part, previous, widget);
					qDebug () << "found" << part << child << previousIndex;
					if (!child)
					{
						QList<QVariant> showData;
						showData << part;
						child = new Util::TreeItem (showData, previous);
						child->ModifyData (0, part, CRRawPath);

						bool inserted = false;
						for (int i = 0; i < previous->ChildCount (); ++i)
						{
							Util::TreeItem *thisChild = previous->Child (i);
							if (part < thisChild->Data (0).toString ())
							{
								qDebug () << "inserting at" << i << previousIndex;
								beginInsertRows (previousIndex, i, i);
								previous->InsertChild (i, child);
								endInsertRows ();
								inserted = true;
								break;
							}
						}

						if (!inserted)
						{
							int pos = rowCount (previousIndex);
							qDebug () << "inserting at (not in loop)" << pos << previousIndex;
							beginInsertRows (previousIndex, pos, pos);
							previous->AppendChild (child);
							endInsertRows ();
						}
					}
					previousIndex = index (previous->ChildPosition (child),
							0, previousIndex);
					previous = child;
				}
				previous->ModifyData (0,
						QVariant::fromValue<QWidget*> (widget), CRWidget);
				Path2Child_ [path] = previous;
				Child2Path_ [previous] = path;
				Widget2Child_ [widget] = previous;
				Child2Widget_ [previous] = widget;
				Pos2Widget_ [idx] = widget;
				Widget2Pos_ [widget] = idx;
			}

			void Core::handleTabRemoved (int idx)
			{
				QWidget *w = Pos2Widget_ [idx];
				Util::TreeItem *child = Widget2Child_ [w];

				QList<int> posHier;
				while (child->Parent ())
				{
					int pos = child->Parent ()->ChildPosition (child);
					posHier.prepend (pos);;
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
				Pos2Widget_.remove (idx);
				Widget2Pos_.remove (w);
			}

			void Core::handleCurrentChanged (int idx)
			{
			}
		};
	};
};

