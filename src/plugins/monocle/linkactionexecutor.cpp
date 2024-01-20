/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "linkactionexecutor.h"
#include <util/sll/visitor.h>
#include "documenttab.h"

namespace LC::Monocle
{
	void ExecuteLinkAction (const LinkAction& action, DocumentTab& tab)
	{
		Util::Visit (action,
				[] (NoAction) {},
				[&] (const NavigationAction& nav) { tab.Navigate (nav); },
				[&] (const ExternalNavigationAction& extNav) { tab.Navigate (extNav); },
				[] (const UrlAction& url) {},
				[] (const CustomAction& custom) { custom (); });
	}
}
