/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "tabspropsmanager.h"
#include <QWidget>
#include <util/xpc/sessionprops.h>

namespace LC::TabSessManager
{
	namespace
	{
		template<typename T>
		Util::DefaultScopeGuard MakePropsGuard (QList<T>& list)
		{
			const auto size = list.size ();
			return Util::MakeScopeGuard ([size, &list]
					{
						if (list.size () > size)
							list.erase (list.begin () + size, list.end ());
					});
		}
	}

	Util::DefaultScopeGuard TabsPropsManager::AppendProps (const TabsProps_t& props)
	{
		auto guard = MakePropsGuard (TabsPropsQueue_);
		TabsPropsQueue_ << props;
		return guard;
	}

	Util::DefaultScopeGuard TabsPropsManager::AppendWindow (int window)
	{
		auto guard = MakePropsGuard (PreferredWindowsQueue_);
		PreferredWindowsQueue_ << window;
		return guard;
	}

	Util::DefaultScopeGuard TabsPropsManager::AppendPosition (int position)
	{
		auto guard = MakePropsGuard (PositionsQueue_);
		PositionsQueue_ << position;
		return guard;
	}

	void TabsPropsManager::HandlePreferredWindowIndex (const IHookProxy_ptr& proxy, const QWidget*)
	{
		if (PreferredWindowsQueue_.empty ())
			return;

		proxy->SetReturnValue (PreferredWindowsQueue_.takeFirst ());
		proxy->CancelDefault ();
	}

	void TabsPropsManager::HandleTabAdding (QWidget *widget)
	{
		if (!TabsPropsQueue_.empty ())
			Util::SetSessionProps (*widget, TabsPropsQueue_.takeFirst ());
		if (!PositionsQueue_.empty ())
		{
			RestoredPositions_ [widget] = PositionsQueue_.takeFirst ();
			connect (widget, &QObject::destroyed, this, [this, widget] { RestoredPositions_.remove (widget); });
		}
	}

	std::optional<int> TabsPropsManager::TakeRestoredPosition (QWidget *widget)
	{
		const auto pos = RestoredPositions_.find (widget);
		if (pos == RestoredPositions_.end ())
			return {};

		const auto position = *pos;
		RestoredPositions_.erase (pos);
		return position;
	}
}
