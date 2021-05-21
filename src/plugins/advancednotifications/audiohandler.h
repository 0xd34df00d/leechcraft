/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QHash>
#include <QDateTime>
#include <interfaces/structures.h>
#include "concretehandlerbase.h"
#include "eventdata.h"

namespace LC::AdvancedNotifications
{
	class AudioThemeManager;

	class AudioHandler : public ConcreteHandlerBase
	{
		const AudioThemeManager * const AudioThemeMgr_;

		QHash<QString, QDateTime> LastNotify_;
	public:
		explicit AudioHandler (const AudioThemeManager*);

		NotificationMethod GetHandlerMethod () const override;
		void Handle (const Entity&, const NotificationRule&) override;
	};
}
