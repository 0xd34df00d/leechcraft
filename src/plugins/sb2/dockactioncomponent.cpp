/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "dockactioncomponent.h"

namespace LC::SB2
{
	DockActionComponent::DockActionComponent (const ICoreProxy_ptr& proxy, SBView *view, QObject *parent)
	: BaseActionComponent ({ "SB2_DockActionImage", "DockComponent.qml", "SB2_dockModel" }, proxy, view, parent)
	{
	}
}
