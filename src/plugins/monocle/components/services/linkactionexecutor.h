/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "linkexecutioncontext.h"

class QMenu;

namespace LC::Monocle
{
	class DocumentTab;

	void ExecuteLinkAction (const LinkAction&, LinkExecutionContext&);
	void AddLinkMenuActions (const LinkAction&, QMenu&, LinkExecutionContext&);
	QString GetLinkActionTooltip (const LinkAction&);
}