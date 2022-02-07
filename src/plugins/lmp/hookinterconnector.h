/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <util/xpc/basehookinterconnector.h>
#include <interfaces/core/ihookproxy.h>

class QMenu;

namespace LC::LMP
{
	struct MediaInfo;

	class HookInterconnector : public Util::BaseHookInterconnector
	{
		Q_OBJECT
	public:
		using Util::BaseHookInterconnector::BaseHookInterconnector;
	signals:
		void hookCollectionContextMenuRequested (LC::IHookProxy_ptr,
				QMenu*,
				const LC::LMP::MediaInfo&);
		void hookPlaylistContextMenuRequested (LC::IHookProxy_ptr,
				QMenu*,
				const LC::LMP::MediaInfo&);
	};
}
