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
	{
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
	}

	ICoreProxy_ptr Core::GetProxy () const
	{
		return Proxy_;
	}

	void Core::SecondInit ()
	{
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
}
}
