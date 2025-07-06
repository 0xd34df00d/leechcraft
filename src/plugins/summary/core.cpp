/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "core.h"
#include <interfaces/ijobholder.h>
#include <util/tags/tagsfiltermodel.h>
#include <util/sll/prelude.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/core/icoretabwidget.h>
#include <interfaces/core/itagsmanager.h>
#include <interfaces/core/irootwindowsmanager.h>
#include <interfaces/core/iiconthememanager.h>
#include "summarywidget.h"
#include "summarytagsfilter.h"

Q_DECLARE_METATYPE (QToolBar*)

namespace LC
{
namespace Summary
{
	Core::Core ()
	: MergeModel_ (new Util::MergeModel ({ {}, {}, {} }))
	, Current_ (nullptr)
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
		delete Current_;
		Current_ = nullptr;
	}

	void Core::SetProxy (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;

		auto rootWM = proxy->GetRootWindowsManager ();
		for (int i = 0; i < rootWM->GetWindowsCount (); ++i)
			handleWindow (i);

		connect (rootWM->GetQObject (),
				SIGNAL (windowAdded (int)),
				this,
				SLOT (handleWindow (int)));

		connect (Proxy_->GetPluginsManager ()->GetQObject (),
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
		for (const auto plugin : Proxy_->GetPluginsManager ()->GetAllCastableTo<IJobHolder*> ())
			MergeModel_->AddModel (plugin->GetRepresentation ());
	}

	bool Core::SameModel (const QModelIndex& i1, const QModelIndex& i2) const
	{
		const auto& mapped1 = MapToSourceRecursively (i1);
		const auto& mapped2 = MapToSourceRecursively (i2);
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

	SummaryTagsFilter* Core::GetTasksModel () const
	{
		const auto filter = new SummaryTagsFilter ();
		filter->setSourceModel (MergeModel_.get ());
		return filter;
	}

	QStringList Core::GetTagsForIndex (int index, QAbstractItemModel *model) const
	{
		int starting = 0;
		const auto merger = dynamic_cast<Util::MergeModel*> (model);
		if (!merger)
		{
			qWarning () << Q_FUNC_INFO
					<< "could not get model" << model;
			return {};
		}
		const auto modIter = merger->GetModelForRow (index, &starting);
		const auto idxModel = *modIter;

		const auto& ids = idxModel->data (idxModel->
				index (index - starting, 0), RoleTags).toStringList ();
		const auto tm = Proxy_->GetTagsManager ();
		return Util::Map (ids, [tm] (const QString& id) { return tm->GetTag (id); });
	}

	QModelIndex Core::MapToSourceRecursively (QModelIndex index) const
	{
		if (!index.isValid ())
			return {};

		while (true)
		{
			const auto model = index.model ();
			if (!model ||
					!model->property ("__LeechCraft_own_core_model").toBool ())
				break;

			if (auto pModel = dynamic_cast<const QAbstractProxyModel*> (model))
			{
				index = pModel->mapToSource (index);
				continue;
			}

			if (auto mModel = dynamic_cast<const Util::MergeModel*> (model))
			{
				index = mModel->mapToSource (index);
				continue;
			}

			qWarning () << Q_FUNC_INFO
					<< "unhandled parent own core model"
					<< model;
			break;
		}

		return index;
	}

	template<typename F>
	SummaryWidget* Core::CreateSummaryWidget (F&& f)
	{
		auto result = new SummaryWidget ();
		std::invoke (f, *result);
		GetProxyHolder ()->GetRootWindowsManager ()->AddTab (tr ("Summary"), result);
		return result;
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

		Current_ = CreateSummaryWidget ([&info] (SummaryWidget& summary)
				{
					for (const auto& pair : info.DynProperties_)
						summary.setProperty (pair.first, pair.second);
					summary.RestoreState (info.Data_);
				});
	}

	void Core::handleNewTabRequested ()
	{
		if (!Current_)
			Current_ = CreateSummaryWidget ([] (auto&&) {});
		else
			emit Current_->raiseTab ();
	}

	void Core::handleWindow (int index)
	{
		auto rootWM = Core::Instance ().GetProxy ()->GetRootWindowsManager ();
		connect (rootWM->GetTabWidget (index)->GetQObject (),
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
