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
#include <QWidgetAction>
#include <QtDebug>
#include <interfaces/ifinder.h>
#include <interfaces/structures.h>
#include <interfaces/ijobholder.h>
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
				connect (Toolbar_,
						SIGNAL (actionTriggered (QAction*)),
						this,
						SLOT (handleActionTriggered (QAction*)));
				ReinitToolbar ();
				Ui_.setupUi (this);

				Core::Instance ().GetProxy ()->GetMainWindow ()->
					addDockWidget (Qt::LeftDockWidgetArea, SearchWidget_);
				SearchWidget_->hide ();

				Q_FOREACH (QObject *plugin, Core::Instance ().GetProxy ()->
						GetPluginsManager ()->GetAllCastableRoots<IFinder*> ())
					connect (plugin,
							SIGNAL (categoriesChanged (const QStringList&,
									const QStringList&)),
							this,
							SLOT (handleCategoriesChanged ()));

				FilterTimer_->setSingleShot (true);
				FilterTimer_->setInterval (800);
				connect (FilterTimer_,
						SIGNAL (timeout ()),
						this,
						SLOT (feedFilterParameters ()));

				Ui_.ControlsDockWidget_->hide ();

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

				QList<QObject*> plugins = Core::Instance ().GetProxy ()->
					GetPluginsManager ()->GetAllCastableRoots<IJobHolder*> ();
				Q_FOREACH (QObject *plugin, plugins)
					ConnectObject (plugin);

				SearchWidget_->SetPossibleCategories (GetUniqueCategories () + QStringList ("downloads"));

				filterParametersChanged ();
			}

			void SummaryWidget::ReconnectModelSpecific ()
			{
				QItemSelectionModel *sel = Ui_.PluginsTasksTree_->selectionModel ();

#define C2(sig,sl,arg1,arg2) \
				if (mo->indexOfMethod (QMetaObject::normalizedSignature ("handleTasksTreeSelection" #sl "(" #arg1 ", " #arg2 ")")) != -1) \
					connect (sel, \
							SIGNAL (sig (arg1, arg2)), \
							object, \
							SLOT (handleTasksTreeSelection##sl (arg1, arg2)));

				QList<QObject*> plugins = Core::Instance ().GetProxy ()->
					GetPluginsManager ()->GetAllCastableRoots<IJobHolder*> ();
				Q_FOREACH (QObject *object, plugins)
				{
					const QMetaObject *mo = object->metaObject ();

					C2 (currentChanged, CurrentChanged, const QModelIndex&, const QModelIndex&);
					C2 (currentColumnChanged, CurrentColumnChanged, const QModelIndex&, const QModelIndex&);
					C2 (currentRowChanged, CurrentRowChanged, const QModelIndex&, const QModelIndex&);
				}
#undef C2
			}

			void SummaryWidget::ConnectObject (QObject *object)
			{
				const QMetaObject *mo = object->metaObject ();

#define C1(sig,sl,arg) \
				if (mo->indexOfMethod (QMetaObject::normalizedSignature ("handleTasksTree" #sl "(" #arg ")")) != -1) \
					connect (Ui_.PluginsTasksTree_, \
							SIGNAL (sig (arg)), \
							object, \
							SLOT (handleTasksTree##sl (arg)));

				C1 (activated, Activated, const QModelIndex&);
				C1 (clicked, Clicked, const QModelIndex&);
				C1 (doubleClicked, DoubleClicked, const QModelIndex&);
				C1 (entered, Entered, const QModelIndex&);
				C1 (pressed, Pressed, const QModelIndex&);
				C1 (viewportEntered, ViewportEntered, );
#undef C1
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
					SearchWidget_->SelectCategories (query);

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

				QString prepend;
				QStringList categories = SearchWidget_->GetCategories ();
				if (categories.size ())
				{
					prepend = QString ("ca:\"%1\"")
						.arg (categories.takeFirst ());
					Q_FOREACH (const QString& cat, categories)
						prepend += QString (" OR ca:\"%1\"").arg (cat);
				}

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
				result.Categories_ = SearchWidget_->GetCategories ();
				result.Op_ = SearchWidget_->IsOr () ?
					Query2::OPOr :
					Query2::OPAnd;
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
				Q_FOREACH (QAction *action, Toolbar_->actions ())
					if (action != ActionSearch_ &&
							!qobject_cast<QWidgetAction*> (action))
						delete action;
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

			void SummaryWidget::handleActionTriggered (QAction *proxyAction)
			{
				if (proxyAction == ActionSearch_)
					return;

				QAction *action = qobject_cast<QAction*> (proxyAction->
						data ().value<QObject*> ());
				QItemSelectionModel *selModel =
						Ui_.PluginsTasksTree_->selectionModel ();
				QModelIndexList indexes = selModel->selectedRows ();
				action->setProperty ("SelectedRows",
						QVariant::fromValue<QList<QModelIndex> > (indexes));
				action->setProperty ("ItemSelectionModel",
						QVariant::fromValue<QObject*> (selModel));

				action->activate (QAction::Trigger);
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
						QList<QAction*> proxies;
						Q_FOREACH (QAction *action, controls->actions ())
						{
							QString ai = action->property ("ActionIcon").toString ();
							if (!ai.isEmpty () &&
									action->icon ().isNull ())
								action->setIcon (Core::Instance ().GetProxy ()->GetIcon (ai));
						}

						Q_FOREACH (QAction *action, controls->actions ())
						{
							QAction *pa = new QAction (action->icon (),
									action->text (), Toolbar_);
							if (action->isSeparator ())
								pa->setSeparator (true);
							else if (qobject_cast<QWidgetAction*> (action))
							{
								proxies << action;
								continue;
							}
							else
							{
								pa->setCheckable (action->isCheckable ());
								pa->setChecked (action->isChecked ());
								pa->setShortcuts (action->shortcuts ());
								pa->setStatusTip (action->statusTip ());
								pa->setToolTip (action->toolTip ());
								pa->setWhatsThis (action->whatsThis ());
								pa->setData (QVariant::fromValue<QObject*> (action));

								connect (pa,
										SIGNAL (hovered ()),
										action,
										SIGNAL (hovered ()));
								connect (pa,
										SIGNAL (toggled (bool)),
										action,
										SIGNAL (toggled (bool)));
							}
							proxies << pa;
						}
						Toolbar_->addActions (proxies);
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
				Util::MergeModel *tasksModel = Core::Instance ().GetTasksModel (query);
				Ui_.PluginsTasksTree_->setModel (tasksModel);
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

				ReconnectModelSpecific ();

				emit changeTabName (newName);
				emit queryUpdated (query);
				emit raiseTab (this);
			}

			void SummaryWidget::on_PluginsTasksTree__customContextMenuRequested (const QPoint& pos)
			{
				QModelIndex current = Ui_.PluginsTasksTree_->currentIndex ();
				QMenu *menu = current.data (RoleContextMenu).value<QMenu*> ();
				if (!menu)
					return;
				menu->popup (Ui_.PluginsTasksTree_->viewport ()->mapToGlobal (pos));
			}

			void SummaryWidget::handleCategoriesChanged ()
			{
				QStringList currentCats = GetUniqueCategories ();

				QStringList currentSelection = SearchWidget_->GetCategories ();
				SearchWidget_->SetPossibleCategories (currentCats + QStringList ("downloads"));
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

