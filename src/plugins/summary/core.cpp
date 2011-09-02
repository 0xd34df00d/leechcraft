/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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
#include <interfaces/ijobholder.h>
#include <util/tagsfiltermodel.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/core/icoretabwidget.h>
#include <interfaces/core/itagsmanager.h>
#include "summarywidget.h"
#include "requestnormalizer.h"
#include "summarytagsfilter.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Summary
		{
			Core::Core ()
			: MergeModel_ (new Util::MergeModel (QStringList (QString ())
						<< QString ()
						<< QString ()))
			, Default_ (0)
			, Current_ (0)
			{
				MergeModel_->setObjectName ("Core MergeModel");
				MergeModel_->setProperty ("__LeechCraft_own_core_model", true);
			}

			Core& Core::Instance ()
			{
				static Core core;
				return core;
			}

			void Core::Release ()
			{
				while (Others_.size ())
					delete Others_.takeFirst ();

				delete Default_;

				KeepProxiesThisWay_.clear ();
			}

			void Core::SetProxy (ICoreProxy_ptr proxy)
			{
				Proxy_ = proxy;
				connect (Proxy_->GetTabWidget ()->GetObject (),
						SIGNAL (currentChanged (int)),
						this,
						SLOT (handleCurrentTabChanged (int)));
				connect (Proxy_->GetPluginsManager ()->GetObject (),
						SIGNAL (pluginInjected (QObject*)),
						this,
						SLOT (handlePluginInjected (QObject*)));
			}

			ICoreProxy_ptr Core::GetProxy () const
			{
				return Proxy_;
			}

			void Core::SecondInit ()
			{
				QList<IJobHolder*> plugins = Proxy_->
					GetPluginsManager ()->GetAllCastableTo<IJobHolder*> ();
				Q_FOREACH (IJobHolder *plugin, plugins)
					MergeModel_->AddModel (plugin->GetRepresentation ());

				Default_ = CreateSummaryWidget ();
				Default_->setProperty ("IsUnremoveable", true);
			}

			SummaryWidget* Core::GetDefaultTab () const
			{
				return Default_;
			}

			QTreeView* Core::GetCurrentView () const
			{
				if (Current_)
					return Current_->GetUi ().PluginsTasksTree_;
				else
					return 0;
			}

			bool Core::CouldHandle (const LeechCraft::Entity& e) const
			{
				return e.Mime_ == "x-leechcraft/category-search-request" &&
						e.Entity_.canConvert<QString> ();
			}

			void Core::Handle (LeechCraft::Entity e)
			{
				const QString& query = e.Entity_.toString ();
				QStringList cats = e.Additional_ ["Categories"].toStringList ();

				SummaryWidget *newTab = CreateSummaryWidget ();

				Others_ << newTab;

				cats.prepend (query);
				newTab->SetQuery (cats);

				emit addNewTab (tr ("Summary"), newTab);
				emit changeTabIcon (newTab, QIcon (":/plugins/summary/resources/images/summary.svg"));
				emit raiseTab (newTab);
			}

			bool Core::SameModel (const QModelIndex& i1, const QModelIndex& i2) const
			{
				const QModelIndex& mapped1 = MapToSourceRecursively (i1);
				const QModelIndex& mapped2 = MapToSourceRecursively (i2);
				return mapped1.model () == mapped2.model ();
			}

			QToolBar* Core::GetControls (const QModelIndex& index) const
			{
				if (!index.isValid ())
					return 0;

				const QVariant& data = index.data (RoleControls);
				return data.value<QToolBar*> ();
			}

			QWidget* Core::GetAdditionalInfo (const QModelIndex& index) const
			{
				if (!index.isValid ())
					return 0;

				const QVariant& data = index.data (RoleAdditionalInfo);
				return data.value<QWidget*> ();
			}

			QAbstractItemModel* Core::GetTasksModel (const QString& text) const
			{
				RequestNormalizer *rm = new RequestNormalizer (MergeModel_);
				rm->SetRequest (text);
				return rm->GetModel ();
			}

			namespace
			{
				template<typename T>
				void FilterByCats (const QSet<QString>& cats,
						QList<T>& finders,
						Query2::Operation operation)
				{
					Q_FOREACH (T finder, finders)
					{
						QSet<QString> fcats = QSet<QString>::fromList (finder->GetCategories ());
						bool exclude = true;
						if (operation == Query2::OPOr)
						{
							if (fcats.intersect (cats).size ())
								exclude = false;
						}
						else
						{
							if (fcats.contains (cats))
								exclude = false;
						}

						if (exclude)
							finders.removeAll (finder);
					}
				}
			};

			Util::MergeModel* Core::GetTasksModel (const Query2& query) const
			{
				QStringList headers = QStringList (tr ("Name"))
					<< tr ("Status")
					<< tr ("State");
				Util::MergeModel *result = new Util::MergeModel (headers);
				connect (result,
						SIGNAL (destroyed (QObject*)),
						this,
						SLOT (handleTaskModelDestroyed ()));
				result->setProperty ("__LeechCraft_own_core_model", true);
				if (query.Categories_.contains ("d") ||
						query.Categories_.contains ("downloads") ||
						query.Categories_.isEmpty ())
				{
					SummaryTagsFilter *filter = new SummaryTagsFilter ();
					filter->setDynamicSortFilter (true);
					filter->setSourceModel (MergeModel_.get ());
					filter->setFilterCaseSensitivity (Qt::CaseInsensitive);

					switch (query.Type_)
					{
					case Query2::TString:
						filter->setFilterFixedString (query.Query_);
						break;
					case Query2::TWildcard:
						filter->setFilterFixedString (query.Query_);
						break;
					case Query2::TRegexp:
						filter->setFilterFixedString (query.Query_);
						break;
					case Query2::TTags:
						filter->setFilterFixedString (query.Query_);
						filter->setTagsMode (true);
						break;
					}

					result->AddModel (filter);
				}

				QList<IFinder*> finders = Proxy_->
					GetPluginsManager ()->GetAllCastableTo<IFinder*> ();
				const QSet<QString>& cats = QSet<QString>::fromList (query.Categories_);
				FilterByCats (cats, finders, query.Op_);

				QSet<QByteArray> used;
				Q_FOREACH (IFinder *finder, finders)
				{
					QList<IFindProxy_ptr> proxies;
					Q_FOREACH (const QString& category, cats)
					{
						Request r =
						{
							false,
							static_cast<Request::Type> (query.Type_),
							QString (),
							category,
							query.Query_,
							query.Params_ [category]
						};

						proxies += finder->GetProxy (r);
					}

					FilterByCats (cats, proxies, query.Op_);

					Q_FOREACH (IFindProxy_ptr proxy, proxies)
					{
						const QByteArray& thisId = proxy->GetUniqueSearchID ();
						if (used.contains (thisId))
							continue;

						used << thisId;
						KeepProxiesThisWay_ [result] << proxy;
						result->AddModel (proxy->GetModel ());
					}
				}

				return result;
			}

			QStringList Core::GetTagsForIndex (int index, QAbstractItemModel *model) const
			{
				int starting = 0;
				Util::MergeModel::const_iterator modIter =
						dynamic_cast<Util::MergeModel*> (model)->GetModelForRow (index, &starting);

				QStringList ids = (*modIter)->data ((*modIter)->
						index (index - starting, 0), RoleTags).toStringList ();
				QStringList result;
				Q_FOREACH (const QString& id, ids)
					result << Proxy_->GetTagsManager ()->GetTag (id);
				return result;
			}

			QModelIndex Core::MapToSourceRecursively (QModelIndex index) const
			{
				if (!index.isValid ())
					return QModelIndex ();

				while (true)
				{
					if (!index.model ()->property ("__LeechCraft_own_core_model").toBool ())
						break;

					const QAbstractProxyModel *pModel = qobject_cast<const QAbstractProxyModel*> (index.model ());
					if (pModel)
					{
						index = pModel->mapToSource (index);
						continue;
					}

					const Util::MergeModel *mModel = qobject_cast<const Util::MergeModel*> (index.model ());
					if (mModel)
					{
						index = mModel->mapToSource (index);
						continue;
					}

					qWarning () << Q_FUNC_INFO
							<< "unhandled parent own core model"
							<< index.model ();
					break;
				}

				return index;
			}

			void Core::MadeCurrent (SummaryWidget *tc)
			{
				Q_FOREACH (SummaryWidget *w, Others_ + (QList<SummaryWidget*> () << Default_))
					w->SmartDeselect (tc);
			}

			SummaryWidget* Core::CreateSummaryWidget ()
			{
				SummaryWidget *result = new SummaryWidget ();
				connect (result,
						SIGNAL (changeTabName (const QString&)),
						this,
						SLOT (handleChangeTabName (const QString&)));
				connect (result,
						SIGNAL (needToClose ()),
						this,
						SLOT (handleNeedToClose ()));
				connect (result,
						SIGNAL (raiseTab (QWidget*)),
						this,
						SIGNAL (raiseTab (QWidget*)));
				return result;
			}

			void Core::handleChangeTabName (const QString& name)
			{
				emit changeTabName (qobject_cast<QWidget*> (sender ()), name);
			}

			void Core::handleCurrentTabChanged (int newIndex)
			{
				QWidget *newTab = Proxy_->GetTabWidget ()->Widget (newIndex);
				Current_ = qobject_cast<SummaryWidget*> (newTab);
				MadeCurrent (Current_);
			}

			void Core::handleNewTabRequested ()
			{
				SummaryWidget *newTab = CreateSummaryWidget ();

				Others_ << newTab;

				emit addNewTab (tr ("Summary"), newTab);
				emit changeTabIcon (newTab, QIcon (":/plugins/summary/resources/images/summary.svg"));
				emit raiseTab (newTab);
			}

			void Core::handleNeedToClose ()
			{
				SummaryWidget *tab = qobject_cast<SummaryWidget*> (sender ());
				if (!tab)
				{
					qWarning () << Q_FUNC_INFO
						<< "not a SummaryWidget*"
						<< sender ();
					return;
				}

				emit removeTab (tab);

				Others_.removeAll (tab);
				tab->deleteLater ();
			}

			void Core::handleTaskModelDestroyed ()
			{
				QAbstractItemModel *model = static_cast<QAbstractItemModel*> (sender ());
				if (!KeepProxiesThisWay_.contains (model))
				{
					qWarning () << Q_FUNC_INFO
						<< sender ()
						<< "doesn't exist";
					return;
				}

				KeepProxiesThisWay_.remove (model);
			}

			void Core::handlePluginInjected (QObject *object)
			{
				IJobHolder *ijh = qobject_cast<IJobHolder*> (object);
				if (ijh)
					MergeModel_->AddModel (ijh->GetRepresentation ());

				IFinder *finder = qobject_cast<IFinder*> (object);
				QList<SummaryWidget*> allsw = Others_;
				allsw << Default_;
				if (finder)
					Q_FOREACH (SummaryWidget *sw, allsw)
					{
						sw->handleCategoriesChanged ();

						connect (object,
								SIGNAL (categoriesChanged (const QStringList&,
										const QStringList&)),
								sw,
								SLOT (handleCategoriesChanged ()));
					}
			}
		};
	};
};

