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
#include "toolbarguard.h"

namespace LeechCraft
{
	TabContents::TabContents (QWidget *parent)
	: QWidget (parent)
	, FilterTimer_ (new QTimer (this))
	, Controls_ (0)
	{
		Ui_.setupUi (this);

		Q_FOREACH (QObject *plugin, Core::Instance ().GetPluginManager ()->
				GetAllCastableRoots<IFinder*> ())
			connect (plugin,
					SIGNAL (categoriesChanged (const QStringList&, const QStringList&)),
					this,
					SLOT (handleCategoriesChanged (const QStringList&, const QStringList&)));

		Ui_.LeastCategory_->setDuplicatesEnabled (true);

		FilterTimer_->setSingleShot (true);
		FilterTimer_->setInterval (800);
		connect (FilterTimer_,
				SIGNAL (timeout ()),
				this,
				SLOT (feedFilterParameters ()));

		Ui_.ControlsDockWidget_->hide ();

		connect (Ui_.SimpleSearch_,
				SIGNAL (toggled (bool)),
				this,
				SLOT (filterParametersChanged ()));
		connect (Ui_.LeastCategory_,
				SIGNAL (currentIndexChanged (int)),
				this,
				SLOT (filterParametersChanged ()));
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

	void TabContents::AllowPlugins ()
	{
		FillCombobox (Ui_.LeastCategory_);
	}

	Ui::TabContents TabContents::GetUi () const
	{
		return Ui_;
	}

	void TabContents::SetQuery (QStringList query)
	{
		if (query.isEmpty ())
			return;

		Ui_.FilterLine_->setText (query.takeFirst ());

		if (!query.isEmpty ())
			Ui_.LeastCategory_->setCurrentIndex (Ui_.LeastCategory_->findText (query.takeFirst ()));

		Q_FOREACH (QString cat, query)
		{
			on_Add__released ();
			QComboBox *box = AdditionalBoxes_.last ();
			box->setCurrentIndex (box->findText (cat));
		}

		feedFilterParameters ();
	}

	void TabContents::FillCombobox (QComboBox *box)
	{
		box->addItem ("downloads");
		Q_FOREACH (IFinder *plugin, Core::Instance ().GetPluginManager ()->
				GetAllCastableTo<IFinder*> ())
			box->addItems (plugin->GetCategories ());
		box->adjustSize ();
	}

	QString TabContents::GetQuery () const
	{
		QString query = Ui_.FilterLine_->text ();
		if (Ui_.SimpleSearch_->checkState () == Qt::Checked)
		{
			QString prepend = QString ("ca:%1")
				.arg (Ui_.LeastCategory_->currentText ());
			Q_FOREACH (QComboBox *box, AdditionalBoxes_)
				prepend += QString (" OR ca:%1").arg (box->currentText ());
			prepend = QString ("(%1) ").arg (prepend);
			prepend += "t:";
			switch (Ui_.Type_->currentIndex ())
			{
				case 0:
					prepend += 'f';
					break;
				case 1:
					prepend += 'w';
					break;
				case 2:
					prepend += 'r';
					break;
				case 3:
					prepend += 't';
					break;
				default:
					prepend += 'f';
					qWarning () << Q_FUNC_INFO
						<< "unknown Type index"
						<< Ui_.Type_->currentIndex ()
						<< Ui_.Type_->currentText ();
					break;
			}

			prepend += ' ';
			query.prepend (prepend);
		}
		return query;
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

	void TabContents::updatePanes (const QModelIndex& newIndex,
			const QModelIndex& oldIndex)
	{
#ifdef QT_DEBUG
		qDebug () << Q_FUNC_INFO;
#endif

		if (oldIndex.isValid () &&
				Core::Instance ().SameModel (newIndex, oldIndex))
		{
		}
		else
		{
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


			QToolBar *controls = Core::Instance ()			
						.GetControls (newIndex);

			QWidget *addiInfo = Core::Instance ()
						.GetAdditionalInfo (newIndex);

#ifdef QT_DEBUG
			qDebug () << "inserting newer stuff" << newIndex << controls << addiInfo;
#endif

			if (controls)
			{
				controls->setFloatable (true);
				controls->setMovable (true);
				Core::Instance ().GetReallyMainWindow ()->
					GetGuard ()->AddToolbar (controls);
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
		QString query = GetQuery ();
		QAbstractItemModel *old = Ui_.PluginsTasksTree_->model ();
		Ui_.PluginsTasksTree_->
			setModel (Core::Instance ().GetTasksModel (query));
		delete old;

		connect (Ui_.PluginsTasksTree_->selectionModel (),
				SIGNAL (currentRowChanged (const QModelIndex&,
						const QModelIndex&)),
				this,
				SLOT (updatePanes (const QModelIndex&,
						const QModelIndex&)));

		QHeaderView *itemsHeader = Ui_.PluginsTasksTree_->header ();
		QFontMetrics fm = fontMetrics ();
		itemsHeader->resizeSection (0,
				fm.width ("Average download job or torrent name is just like this."));
		itemsHeader->resizeSection (1,
				fm.width ("Of the download."));
		itemsHeader->resizeSection (2,
				fm.width ("99.99% (1024.0 kb from 1024.0 kb at 1024.0 kb/s)"));

		emit filterUpdated ();
		emit queryUpdated (query);
	}

	void TabContents::on_PluginsTasksTree__customContextMenuRequested (const QPoint& pos)
	{
		QModelIndex current = Ui_.PluginsTasksTree_->currentIndex ();
		QMenu *menu = current.data (RoleContextMenu).value<QMenu*> ();
		if (!menu)
			return;
		menu->popup (Ui_.PluginsTasksTree_->viewport ()->mapToGlobal (pos));
	}
	
	void TabContents::on_Add__released ()
	{
		QComboBox *box = new QComboBox (this);
		box->setDuplicatesEnabled (true);
		box->setInsertPolicy (QComboBox::InsertAlphabetically);
		connect (box,
				SIGNAL (currentIndexChanged (int)),
				this,
				SLOT (filterParametersChanged ()));

		FillCombobox (box);

		QAction *remove = new QAction (tr ("Remove this category"), this);
		connect (remove,
				SIGNAL (triggered ()),
				this,
				SLOT (removeCategoryBox ()));
		remove->setData (QVariant::fromValue<QWidget*> (box));
		box->setContextMenuPolicy (Qt::ActionsContextMenu);
		box->addAction (remove);

		Ui_.SearchStuff_->insertWidget (3, box);
		AdditionalBoxes_ << box;
	}

	void TabContents::handleCategoriesChanged (const QStringList& newCats, const QStringList& oldCats)
	{
		Q_FOREACH (QComboBox *box, AdditionalBoxes_ + (QList<QComboBox*> () << Ui_.LeastCategory_))
		{
			Q_FOREACH (QString category, oldCats)
				box->removeItem (box->findText (category));
			box->addItems (newCats);
		}
	}

	void TabContents::on_SimpleSearch__toggled (bool on)
	{
		Q_FOREACH (QComboBox *box, AdditionalBoxes_)
			box->setEnabled (on);
	}

	void TabContents::removeCategoryBox ()
	{
		QAction *act = qobject_cast<QAction*> (sender ());
		if (!act)
		{
			qWarning () << Q_FUNC_INFO
				<< "sender is not a QAction*"
				<< sender ();
			return;
		}

		QComboBox *w = qobject_cast<QComboBox*> (act->data ().value<QWidget*> ());
		AdditionalBoxes_.removeAll (w);
		w->deleteLater ();

		filterParametersChanged ();
	}
};

