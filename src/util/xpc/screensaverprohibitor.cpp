/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "screensaverprohibitor.h"
#include <QString>
#include <QUuid>
#include <interfaces/core/ientitymanager.h>
#include "util.h"

namespace LC::Util
{
	ScreensaverProhibitor::ScreensaverProhibitor (IEntityManager *iem)
	: IEM_ { iem }
	, ContextID_ { QUuid::createUuid ().toString () }
	{
	}

	ScreensaverProhibitor::~ScreensaverProhibitor ()
	{
		SetProhibited (false);
	}

	void ScreensaverProhibitor::SetProhibited (bool prohibited)
	{
		if (Prohibited_ == prohibited)
			return;

		Prohibited_ = prohibited;

		if (Enabled_)
			SendEntity (Prohibited_);
	}

	void ScreensaverProhibitor::SetProhibitionsEnabled (bool enabled)
	{
		if (Enabled_ == enabled)
			return;

		Enabled_ = enabled;

		if (Prohibited_)
			SendEntity (Enabled_);
	}

	void ScreensaverProhibitor::SendEntity (bool prohibit)
	{
		auto e = MakeEntity ("ScreensaverProhibition", {}, {}, "x-leechcraft/power-management");
		e.Additional_ = {
				{ "Enable", prohibit },
				{ "ContextID", ContextID_ }
			};
		IEM_->HandleEntity (e);
	}
}
