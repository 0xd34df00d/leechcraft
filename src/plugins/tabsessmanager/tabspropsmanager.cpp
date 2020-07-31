/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "tabspropsmanager.h"
#include <QWidget>

namespace LC
{
namespace TabSessManager
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

	void TabsPropsManager::HandlePreferredWindowIndex (const IHookProxy_ptr& proxy, const QWidget*)
	{
		if (PreferredWindowsQueue_.empty ())
			return;

		proxy->SetReturnValue (PreferredWindowsQueue_.takeFirst ());
		proxy->CancelDefault ();
	}

	void TabsPropsManager::HandleTabAdding (QWidget *widget)
	{
		if (TabsPropsQueue_.empty ())
			return;

		for (const auto& pair : TabsPropsQueue_.takeFirst ())
			widget->setProperty (pair.first, pair.second);
	}
}
}
