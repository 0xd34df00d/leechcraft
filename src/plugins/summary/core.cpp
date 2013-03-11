/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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
#include <util/tags/tagsfiltermodel.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/core/icoretabwidget.h>
#include <interfaces/core/itagsmanager.h>
#include <interfaces/core/irootwindowsmanager.h>
#include "summarywidget.h"
#include "summarytagsfilter.h"

namespace LeechCraft
{
namespace Summary
{
	Core::Core ()
	: MergeModel_ (new Util::MergeModel (QStringList (QString ())
				<< QString ()
				<< QString ()))
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
		if (Current_)
			delete Current_;
	}

	void Core::SetProxy (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;

		auto rootWM = proxy->GetRootWindowsManager ();
		for (int i = 0; i < rootWM->GetWindowsCount (); ++i)
			handleWindow (i);

		connect (rootWM->GetObject (),
				SIGNAL (windodwAdded (int)),
				this,
				SLOT (handleWindow (int)));

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
	}

	QTreeView* Core::GetCurrentView () const
	{
		return Current_ ? Current_->GetUi ().PluginsTasksTree_ : 0;
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

	QSortFilterProxyModel* Core::GetTasksModel () const
	{
		SummaryTagsFilter *filter = new SummaryTagsFilter ();
		filter->setProperty ("__LeechCraft_own_core_model", true);
		filter->setDynamicSortFilter (true);
		filter->setSourceModel (MergeModel_.get ());
		filter->setFilterCaseSensitivity (Qt::CaseInsensitive);
		return filter;
	}

	QStringList Core::GetTagsForIndex (int index, QAbstractItemModel *model) const
	{
		// TODO check this — passed model could be not MergeModel anymore.
		int starting = 0;
		auto merger = dynamic_cast<Util::MergeModel*> (model);
		if (!merger)
		{
			qWarning () << Q_FUNC_INFO
					<< "could not get model" << model;
			return QStringList ();
		}
		auto modIter = merger->GetModelForRow (index, &starting);

		QStringList ids = (*modIter)->data ((*modIter)->
				index (index - starting, 0), RoleTags).toStringList ();
		QStringList result;
		Q_FOREACH (const QString& id, ids)
			result << Proxy_->GetTagsManager ()->GetTag (id);
		return result;
	}

	QModelIndex Core::MapToSourceRecursively (QModelIndex index) const
	{
		// TODO as in GetTagsForIndex();
		if (!index.isValid ())
			return QModelIndex ();

		while (true)
		{
			if (!index.model ()->property ("__LeechCraft_own_core_model").toBool ())
				break;

			auto pModel = qobject_cast<const QAbstractProxyModel*> (index.model ());
			if (pModel)
			{
				index = pModel->mapToSource (index);
				continue;
			}

			auto mModel = qobject_cast<const Util::MergeModel*> (index.model ());
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

	SummaryWidget* Core::CreateSummaryWidget ()
	{
		if (Current_)
			return Current_;

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
		emit changeTabName (Current_, name);
	}

	void Core::handleCurrentTabChanged (int newIndex)
	{
		if (!Current_)
			return;

		auto tw = qobject_cast<ICoreTabWidget*> (sender ());
		if (!tw)
		{
			qWarning () << Q_FUNC_INFO
					<< "sender is not a tab widget:"
					<< sender ();
			return;
		}

		auto newTab = tw->Widget (newIndex);
		Current_->SetUpdatesEnabled (Current_ == newTab);
	}

	void Core::RecoverTabs (const QList<TabRecoverInfo>& infos)
	{
		if (infos.isEmpty () || Current_)
			return;

		const auto& info = infos.first ();

		Current_ = CreateSummaryWidget ();

		Q_FOREACH (const auto& pair, info.DynProperties_)
			Current_->setProperty (pair.first, pair.second);

		Current_->RestoreState (info.Data_);

		emit addNewTab (tr ("Summary"), Current_);
		emit changeTabIcon (Current_, QIcon (":/plugins/summary/resources/images/summary.svg"));
	}

	void Core::handleNewTabRequested ()
	{
		if (Current_)
		{
			emit raiseTab (Current_);
			return;
		}

		Current_ = CreateSummaryWidget ();

		emit addNewTab (tr ("Summary"), Current_);
		emit changeTabIcon (Current_, QIcon (":/plugins/summary/resources/images/summary.svg"));
		emit raiseTab (Current_);
	}

	void Core::handleNeedToClose ()
	{
		emit removeTab (Current_);
		Current_->deleteLater ();
		Current_ = 0;
	}

	void Core::handleWindow (int index)
	{
		auto rootWM = Core::Instance ().GetProxy ()->GetRootWindowsManager ();
		connect (rootWM->GetTabWidget (index)->GetObject (),
				SIGNAL (currentChanged (int)),
				this,
				SLOT (handleCurrentTabChanged (int)));
	}

	void Core::handlePluginInjected (QObject *object)
	{
		if (auto ijh = qobject_cast<IJobHolder*> (object))
			MergeModel_->AddModel (ijh->GetRepresentation ());
	}
}
}
