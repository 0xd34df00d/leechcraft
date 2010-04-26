/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2009  Georg Rudoy
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

#include "summarywidget.h"
#include <QTimer>
#include <QComboBox>
#include <QMenu>
#include <QToolBar>
#include <QMainWindow>
#include <QtDebug>
#include <interfaces/ifinder.h>
#include <interfaces/structures.h>
#include "core.h"
#include "searchwidget.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Summary
		{
			QObject *SummaryWidget::S_ParentMultiTabs_ = 0;

			SummaryWidget::SummaryWidget (QWidget *parent)
			: QWidget (parent)
			, FilterTimer_ (new QTimer)
			, Toolbar_ (new QToolBar)
			, SearchWidget_ (new SearchWidget)
			{
				ActionSearch_ = SearchWidget_->toggleViewAction ();
				ActionSearch_->setProperty ("ActionIcon", "find");
				ActionSearch_->setShortcut (tr ("Ctrl+F"));
				Toolbar_->setWindowTitle ("Summary");
				ReinitToolbar ();
				Ui_.setupUi (this);

				Core::Instance ().GetProxy ()->GetMainWindow ()->
					addDockWidget (Qt::LeftDockWidgetArea, SearchWidget_);
				SearchWidget_->hide ();
				connect (SearchWidget_,
						SIGNAL (categoryComboboxRequested ()),
						this,
						SLOT (addCategoryBox ()));

				Q_FOREACH (QObject *plugin, Core::Instance ().GetProxy ()->
						GetPluginsManager ()->GetAllCastableRoots<IFinder*> ())
					connect (plugin,
							SIGNAL (categoriesChanged (const QStringList&, const QStringList&)),
							this,
							SLOT (handleCategoriesChanged (const QStringList&, const QStringList&)));

				FillCombobox (SearchWidget_->GetLeastCategory ());

				FilterTimer_->setSingleShot (true);
				FilterTimer_->setInterval (800);
				connect (FilterTimer_,
						SIGNAL (timeout ()),
						this,
						SLOT (feedFilterParameters ()));

				Ui_.ControlsDockWidget_->hide ();

				connect (SearchWidget_->GetLeastCategory (),
						SIGNAL (currentIndexChanged (int)),
						this,
						SLOT (filterParametersChanged ()));
				connect (SearchWidget_->GetFilterLine (),
						SIGNAL (textEdited (const QString&)),
						this,
						SLOT (filterParametersChanged ()));
				connect (SearchWidget_,
						SIGNAL (paramsChanged ()),
						this,
						SLOT (filterParametersChanged ()));
				connect (SearchWidget_->GetFilterLine (),
						SIGNAL (returnPressed ()),
						this,
						SLOT (filterReturnPressed ()));

				filterParametersChanged ();
			}

			SummaryWidget::~SummaryWidget ()
			{
				Toolbar_->clear ();

				QWidget *widget = Ui_.ControlsDockWidget_->widget ();
				Ui_.ControlsDockWidget_->setWidget (0);
				if (widget)
					widget->setParent (0);

				delete SearchWidget_;
			}

			void SummaryWidget::SetParentMultiTabs (QObject *parent)
			{
				S_ParentMultiTabs_ = parent;
			}

			void SummaryWidget::Remove ()
			{
				emit needToClose ();
			}

			QToolBar* SummaryWidget::GetToolBar () const
			{
				return Toolbar_;
			}

			void SummaryWidget::NewTabRequested ()
			{
				emit newTabRequested ();
			}

			QList<QAction*> SummaryWidget::GetTabBarContextMenuActions () const
			{
				return QList<QAction*> ();
			}

			QObject* SummaryWidget::ParentMultiTabs () const
			{
				return S_ParentMultiTabs_;
			}

			void SummaryWidget::SetQuery (QStringList query)
			{
				if (query.isEmpty ())
					return;

				SearchWidget_->GetFilterLine ()->setText (query.takeFirst ());

				if (!query.isEmpty ())
					SearchWidget_->GetLeastCategory ()->setCurrentIndex (SearchWidget_->GetLeastCategory ()->
							findText (query.takeFirst ()));

				Q_FOREACH (QString cat, query)
				{
					addCategoryBox ();
					QComboBox *box = AdditionalBoxes_.last ();
					box->setCurrentIndex (box->findText (cat));
				}

				feedFilterParameters ();
			}

			QStringList SummaryWidget::GetUniqueCategories () const
			{
				QStringList result;
				Q_FOREACH (IFinder *plugin, Core::Instance ().GetProxy ()->
						GetPluginsManager ()->GetAllCastableTo<IFinder*> ())
					result += plugin->GetCategories ();
				result.removeDuplicates ();
				result.sort ();
				return result;
			}

			void SummaryWidget::FillCombobox (QComboBox *box)
			{
				box->addItem ("downloads");
				box->addItems (GetUniqueCategories ());
				box->adjustSize ();
			}

			QString SummaryWidget::GetQuery () const
			{
				QString query = SearchWidget_->GetFilterLine ()->text ();
				QString prepend = QString ("ca:\"%1\"")
					.arg (SearchWidget_->GetLeastCategory ()->currentText ());
				Q_FOREACH (QComboBox *box, AdditionalBoxes_)
					prepend += QString (" OR ca:\"%1\"").arg (box->currentText ());
				prepend = QString ("(%1) ").arg (prepend);
				prepend += "t:";
				switch (SearchWidget_->GetFilterType ()->currentIndex ())
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
							<< SearchWidget_->GetFilterType ()->currentIndex ()
							<< SearchWidget_->GetFilterType ()->currentText ();
						break;
				}

				prepend += ' ';
				query.prepend (prepend);
				return query;
			}

			Query2 SummaryWidget::GetQuery2 () const
			{
				Query2 result;
				result.Query_ = SearchWidget_->GetFilterLine ()->text ();
				result.Categories_ << (SearchWidget_->GetLeastCategory ()->currentText ());
				result.Op_ = SearchWidget_->IsOr () ?
					Query2::OPOr :
					Query2::OPAnd;;
				Q_FOREACH (QComboBox *box, AdditionalBoxes_)
					result.Categories_ << box->currentText ();
				switch (SearchWidget_->GetFilterType ()->currentIndex ())
				{
					case 0:
						result.Type_ = Query2::TString;
						break;
					case 1:
						result.Type_ = Query2::TWildcard;
						break;
					case 2:
						result.Type_ = Query2::TRegexp;
						break;
					case 3:
						result.Type_ = Query2::TTags;
						break;
					default:
						result.Type_ = Query2::TString;
						qWarning () << Q_FUNC_INFO
							<< "unknown Type index"
							<< SearchWidget_->GetFilterType ()->currentIndex ()
							<< SearchWidget_->GetFilterType ()->currentText ();
						break;
				}

				return result;
			}

			void SummaryWidget::ReinitToolbar ()
			{
				Toolbar_->clear ();
				Toolbar_->addAction (ActionSearch_);
				Toolbar_->addSeparator ();
			}

			void SummaryWidget::SmartDeselect (SummaryWidget *newFocus)
			{
#ifdef QT_DEBUG
				qDebug () << "SmartDeselect" << newFocus << this;
#endif
				if (newFocus &&
						newFocus != this)
				{
					if (Ui_.PluginsTasksTree_->selectionModel ())
					{
						Ui_.PluginsTasksTree_->selectionModel ()->clear ();
						Ui_.PluginsTasksTree_->selectionModel ()->clearSelection ();
						Ui_.PluginsTasksTree_->selectionModel ()->
							setCurrentIndex (QModelIndex (), QItemSelectionModel::ClearAndSelect);
					}
					ReinitToolbar ();
					Ui_.ControlsDockWidget_->hide ();
				}
			}

			Ui::SummaryWidget SummaryWidget::GetUi () const
			{
				return Ui_;
			}

			void SummaryWidget::updatePanes (const QModelIndex& newIndex,
					const QModelIndex& oldIndex)
			{
				QToolBar *controls = Core::Instance ()
							.GetControls (newIndex);

				QWidget *addiInfo = Core::Instance ()
							.GetAdditionalInfo (newIndex);

				if (oldIndex.isValid () &&
						addiInfo != Ui_.ControlsDockWidget_->widget ())
					Ui_.ControlsDockWidget_->hide ();

				if (newIndex.isValid ())
				{
					ReinitToolbar ();
					if (controls)
					{
						Q_FOREACH (QAction *action, controls->actions ())
						{
							QString ai = action->property ("ActionIcon").toString ();
							if (!ai.isEmpty () &&
									action->icon ().isNull ())
								action->setIcon (Core::Instance ().GetProxy ()->GetIcon (ai));
						}
						Toolbar_->addActions (controls->actions ());
					}
					if (addiInfo != Ui_.ControlsDockWidget_->widget ())
						Ui_.ControlsDockWidget_->setWidget (addiInfo);

					if (addiInfo)
						Ui_.ControlsDockWidget_->show ();
				}
			}

			void SummaryWidget::filterParametersChanged ()
			{
				FilterTimer_->stop ();
				FilterTimer_->start ();
			}

			void SummaryWidget::filterReturnPressed ()
			{
				FilterTimer_->stop ();
				feedFilterParameters ();
			}

			void SummaryWidget::feedFilterParameters ()
			{
				QItemSelectionModel *selection = Ui_.PluginsTasksTree_->selectionModel ();
				if (selection)
					selection->setCurrentIndex (QModelIndex (), QItemSelectionModel::Clear);

				Query2 query = GetQuery2 ();
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
				connect (Ui_.PluginsTasksTree_->selectionModel (),
						SIGNAL (currentRowChanged (const QModelIndex&,
								const QModelIndex&)),
						this,
						SLOT (syncSelection (const QModelIndex&)),
						Qt::QueuedConnection);

				QHeaderView *itemsHeader = Ui_.PluginsTasksTree_->header ();
				QFontMetrics fm = fontMetrics ();
				itemsHeader->resizeSection (0,
						fm.width ("Average download job or torrent name is just like this."));
				itemsHeader->resizeSection (1,
						fm.width ("Of the download."));
				itemsHeader->resizeSection (2,
						fm.width ("99.99% (1024.0 kb from 1024.0 kb at 1024.0 kb/s)"));

				QString newName;
				if (query.Query_.size ())
					newName = tr ("S: %1 [%2]")
							.arg (query.Query_)
							.arg (query.Categories_.join ("; "));
				else
					newName = tr ("Summary [%1]")
							.arg (query.Categories_.join ("; "));
				emit changeTabName (newName);
				emit filterUpdated ();
				emit queryUpdated (query);
			}

			void SummaryWidget::on_PluginsTasksTree__customContextMenuRequested (const QPoint& pos)
			{
				QModelIndex current = Ui_.PluginsTasksTree_->currentIndex ();
				QMenu *menu = current.data (RoleContextMenu).value<QMenu*> ();
				if (!menu)
					return;
				menu->popup (Ui_.PluginsTasksTree_->viewport ()->mapToGlobal (pos));
			}
			
			void SummaryWidget::addCategoryBox ()
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

				SearchWidget_->AddCategory (box);
				AdditionalBoxes_ << box;
			}

			void SummaryWidget::handleCategoriesChanged (const QStringList&, const QStringList&)
			{
				QStringList currentCats = GetUniqueCategories ();

				Q_FOREACH (QComboBox *box,
						AdditionalBoxes_ + (QList<QComboBox*> () << SearchWidget_->GetLeastCategory ()))
				{
					box->clear ();
					box->addItem ("downloads");
					box->addItems (currentCats);
				}
			}

			void SummaryWidget::removeCategoryBox ()
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

			void SummaryWidget::syncSelection (const QModelIndex& current)
			{
				QItemSelectionModel *selm = Ui_.PluginsTasksTree_->selectionModel ();
				QModelIndex now = selm->currentIndex ();
#ifdef QT_DEBUG
				qDebug () << Q_FUNC_INFO << this << current << now;
#endif
				if (current != now ||
						(now.isValid () &&
						 !selm->rowIntersectsSelection (now.row (), QModelIndex ())))
				{
					selm->select (now, QItemSelectionModel::ClearAndSelect |
							QItemSelectionModel::Rows);
					updatePanes (now, current);
				}
			}
		};
	};
};

