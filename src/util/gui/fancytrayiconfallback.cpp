/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "fancytrayiconfallback.h"

namespace LC::Util
{
	FancyTrayIconFallback::FancyTrayIconFallback (FancyTrayIcon& icon)
	: FancyTrayIconImpl { &icon }
	, FTI_ { icon }
	{
		connect (&Icon_,
				&QSystemTrayIcon::activated,
				[&icon] (QSystemTrayIcon::ActivationReason reason)
				{
					switch (reason)
					{
					case QSystemTrayIcon::Trigger:
						emit icon.activated ();
						break;
					case QSystemTrayIcon::MiddleClick:
						emit icon.secondaryActivated ();
						break;
					default:
						break;
					}
				});
	}

	void FancyTrayIconFallback::UpdateIcon ()
	{
		Icon_.setIcon (FTI_.GetIcon ());
	}

	void FancyTrayIconFallback::UpdateTooltip ()
	{
		Icon_.setToolTip (FTI_.GetTooltip ().HTML_);
	}

	void FancyTrayIconFallback::UpdateMenu ()
	{
		Icon_.setContextMenu (FTI_.GetContextMenu ());
	}
}
