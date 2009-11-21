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

#include "core.h"
#include <interfaces/ijobholder.h>
#include "summarywidget.h"
#include "requestnormalizer.h"
#include "treeviewreemitter.h"

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
			, Reemitter_ (new TreeViewReemitter (this))
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
				qDeleteAll (Others_);

				delete Default_;
			}

			void Core::SetProxy (ICoreProxy_ptr proxy)
			{
				Proxy_ = proxy;
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

				emit addNewTab (tr ("Summary"), Default_);
				emit changeTabIcon (Default_, QIcon (":/resources/images/summary.svg"));
			}

			TreeViewReemitter* Core::GetTreeViewReemitter () const
			{
				return Reemitter_;
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

			bool Core::SameModel (const QModelIndex& i1, const QModelIndex& i2) const
			{
				QModelIndex mapped1 = MapToSourceRecursively (i1);
				QModelIndex mapped2 = MapToSourceRecursively (i2);
				return mapped1.model () == mapped2.model ();
			}

			QToolBar* Core::GetControls (const QModelIndex& index) const
			{
				if (!index.isValid ())
					return 0;

				QVariant data = index.data (RoleControls);
				return data.value<QToolBar*> ();
			}

			QWidget* Core::GetAdditionalInfo (const QModelIndex& index) const
			{
				if (!index.isValid ())
					return 0;

				QVariant data = index.data (RoleAdditionalInfo);
				return data.value<QWidget*> ();
			}

			QAbstractItemModel* Core::GetTasksModel (const QString& text) const
			{
				RequestNormalizer *rm = new RequestNormalizer (MergeModel_);
				rm->SetRequest (text);
				return rm->GetModel ();
			}

			QStringList Core::GetTagsForIndex (int index, QAbstractItemModel *model) const
			{
				Util::MergeModel::const_iterator modIter =
					dynamic_cast<Util::MergeModel*> (model)->GetModelForRow (index);

				int starting = dynamic_cast<Util::MergeModel*> (model)->
					GetStartingRow (modIter);

				QStringList ids = (*modIter)->data ((*modIter)->
						index (index - starting, 0), RoleTags).toStringList ();
				QStringList result;
				Q_FOREACH (QString id, ids)
					result << Proxy_->GetTagsManager ()->GetTag (id);
				return result;
			}

			QModelIndex Core::MapToSourceRecursively (QModelIndex index) const
			{
				const QAbstractProxyModel *model = 0;
				bool mapped = false;
				while ((model = qobject_cast<const QAbstractProxyModel*> (index.model ())) &&
						model->property ("__LeechCraft_own_core_model").toBool ())
				{
					index = model->mapToSource (index);
					mapped = true;
				}

				if (!mapped)
					return QModelIndex ();

				return index;
			}

			SummaryWidget* Core::CreateSummaryWidget ()
			{
				SummaryWidget *result = new SummaryWidget ();
				connect (result,
						SIGNAL (newTabRequested ()),
						this,
						SLOT (handleNewTabRequested ()));
				connect (result,
						SIGNAL (needToClose ()),
						this,
						SLOT (handleNeedToClose ()));
				connect (result,
						SIGNAL (filterUpdated ()),
						this,
						SLOT (handleFilterUpdated ()));
				Reemitter_->Connect (result);
				return result;
			}

			void Core::handleCurrentTabChanged (int newIndex)
			{
				QWidget *newTab = Proxy_->GetTabWidget ()->widget (newIndex);
				Current_ = qobject_cast<SummaryWidget*> (newTab);
			}

			void Core::handleNewTabRequested ()
			{
				SummaryWidget *newTab = CreateSummaryWidget ();

				Others_ << newTab;

				emit addNewTab (tr ("Summary"), newTab);
				emit changeTabIcon (newTab, QIcon (":/resources/images/summary.svg"));
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

			void Core::handleFilterUpdated ()
			{
				SummaryWidget *w = qobject_cast<SummaryWidget*> (sender ());
				if (!w)
				{
					qDebug () << Q_FUNC_INFO
						<< "not a SummaryWidget*"
						<< sender ();
					return;
				}

				Reemitter_->ConnectModelSpecific (w);
			}
		};
	};
};

