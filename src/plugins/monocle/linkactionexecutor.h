/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "interfaces/monocle/ilink.h"

class QMenu;

namespace LC::Monocle
{
	class DocumentTab;

	struct LinkExecutionContext
	{
		LinkExecutionContext () = default;

		LinkExecutionContext (const LinkExecutionContext&) = delete;
		LinkExecutionContext (LinkExecutionContext&&) = delete;
		LinkExecutionContext& operator= (const LinkExecutionContext&) = delete;
		LinkExecutionContext& operator= (LinkExecutionContext&&) = delete;

		virtual void Navigate (const NavigationAction&) = 0;
		virtual void Navigate (const ExternalNavigationAction&) = 0;
	protected:
		// Objects of derived classes are not intended to be destroyed via
		// `delete` through the pointer to them.
		~LinkExecutionContext () = default;
	};

	void ExecuteLinkAction (const LinkAction&, LinkExecutionContext&);
	void AddLinkMenuActions (const LinkAction&, QMenu&, LinkExecutionContext&);
	QString GetLinkActionTooltip (const LinkAction&);
}
