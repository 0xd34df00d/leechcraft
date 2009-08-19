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

#include "tabcontents.h"
#include <QTimer>
#include <QMenu>
#include "core.h"
#include "mainwindow.h"

namespace LeechCraft
{
	TabContents::TabContents (QWidget *parent)
	: QWidget (parent)
	, FilterTimer_ (new QTimer (this))
	, Controls_ (0)
	{
		Ui_.setupUi (this);

		FilterTimer_->setSingleShot (true);
		FilterTimer_->setInterval (800);
		connect (FilterTimer_,
				SIGNAL (timeout ()),
				this,
				SLOT (feedFilterParameters ()));

		Ui_.ControlsDockWidget_->hide ();

		connect (Ui_.FilterLine_,
				SIGNAL (textEdited (const QString&)),
				this,
				SLOT (filterParametersChanged ()));
		connect (Ui_.FilterLine_,
				SIGNAL (returnPressed ()),
				this,
				SLOT (filterReturnPressed ()));

		filterParametersChanged ();
	}

	TabContents::~TabContents ()
	{
		QWidget *widget = Ui_.ControlsDockWidget_->widget ();
		Ui_.ControlsDockWidget_->setWidget (0);
		widget->setParent (0);

		QAbstractItemModel *old = Ui_.PluginsTasksTree_->model ();
		Ui_.PluginsTasksTree_->setModel (0);
		delete old;
	}

	Ui::TabContents TabContents::GetUi () const
	{
		return Ui_;
	}

	void TabContents::SetQuery (const QString& query)
	{
		Ui_.FilterLine_->setText (query);
		feedFilterParameters ();
	}

	void TabContents::SmartDeselect (TabContents *newFocus)
	{
		if (Controls_)
			Core::Instance ().GetReallyMainWindow ()->
				removeToolBar (Controls_);

		if (newFocus &&
				Ui_.PluginsTasksTree_->selectionModel ())
			Ui_.PluginsTasksTree_->selectionModel ()->clear ();
	}

	void TabContents::updatePanes (const QItemSelection& newIndexes,
			const QItemSelection& oldIndexes)
	{
#ifdef QT_DEBUG
		qDebug () << Q_FUNC_INFO;
#endif

		QModelIndex oldIndex, newIndex;
		if (oldIndexes.size ())
			oldIndex = oldIndexes.at (0).topLeft ();
		if (newIndexes.size ())
			newIndex = newIndexes.at (0).topLeft ();

		if (!newIndex.isValid ())
		{
#ifdef QT_DEBUG
			qDebug () << "invalidating";
#endif

			if (oldIndex.isValid ())
			{
#ifdef QT_DEBUG
				qDebug () << "erasing older stuff";
#endif
				QToolBar *oldControls = Core::Instance ().GetControls (oldIndex);
				if (oldControls)
					Core::Instance ().GetReallyMainWindow ()->
						removeToolBar (oldControls);
				Ui_.ControlsDockWidget_->hide ();
			}
		}
		else if (oldIndex.isValid () &&
				Core::Instance ().SameModel (newIndex, oldIndex))
		{
		}
		else if (newIndex.isValid ())
		{
			QToolBar *controls = Core::Instance ()
						.GetControls (newIndex);
			QWidget *addiInfo = Core::Instance ()
						.GetAdditionalInfo (newIndex);

			if (oldIndex.isValid ())
			{
#ifdef QT_DEBUG
				qDebug () << "erasing older stuff";
#endif
				QToolBar *oldControls = Core::Instance ().GetControls (oldIndex);
				if (oldControls)
					Core::Instance ().GetReallyMainWindow ()->
						removeToolBar (oldControls);
				Ui_.ControlsDockWidget_->hide ();
			}

#ifdef QT_DEBUG
			qDebug () << "inserting newer stuff" << newIndex << controls << addiInfo;
#endif

			if (controls)
			{
				controls->setFloatable (true);
				controls->setMovable (true);
				Core::Instance ().GetReallyMainWindow ()->
					addToolBar (controls);
				controls->show ();
				Controls_ = controls;
			}
			if (addiInfo)
			{
				if (addiInfo->parent () != this)
					addiInfo->setParent (this);
				Ui_.ControlsDockWidget_->setWidget (addiInfo);
				Ui_.ControlsDockWidget_->show ();
			}
		}
	}

	void TabContents::filterParametersChanged ()
	{
		FilterTimer_->stop ();
		FilterTimer_->start ();
	}

	void TabContents::filterReturnPressed ()
	{
		FilterTimer_->stop ();
		feedFilterParameters ();
	}

	void TabContents::feedFilterParameters ()
	{
		QAbstractItemModel *old = Ui_.PluginsTasksTree_->model ();
		Ui_.PluginsTasksTree_->
			setModel (Core::Instance ()
					.GetTasksModel (Ui_.FilterLine_->text ()));
		delete old;

		connect (Ui_.PluginsTasksTree_->selectionModel (),
				SIGNAL (selectionChanged (const QItemSelection&,
						const QItemSelection&)),
				this,
				SLOT (updatePanes (const QItemSelection&,
						const QItemSelection&)));

		QHeaderView *itemsHeader = Ui_.PluginsTasksTree_->header ();
		QFontMetrics fm = fontMetrics ();
		itemsHeader->resizeSection (0,
				fm.width ("Average download job or torrent name is just like this."));
		itemsHeader->resizeSection (1,
				fm.width ("Of the download."));
		itemsHeader->resizeSection (2,
				fm.width ("99.99% (1024.0 kb from 1024.0 kb at 1024.0 kb/s)"));

		emit filterUpdated ();
	}

	void TabContents::on_PluginsTasksTree__customContextMenuRequested (const QPoint& pos)
	{
		QModelIndex current = Ui_.PluginsTasksTree_->currentIndex ();
		QMenu *menu = current.data (RoleContextMenu).value<QMenu*> ();
		if (!menu)
			return;
		menu->popup (Ui_.PluginsTasksTree_->viewport ()->mapToGlobal (pos));
	}
};

