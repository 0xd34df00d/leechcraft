/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "audiohandler.h"
#include <util/xpc/util.h>
#include <util/sys/resourceloader.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ientitymanager.h>
#include "generalhandler.h"
#include "xmlsettingsmanager.h"
#include "audiothememanager.h"
#include "notificationrule.h"

namespace LC::AdvancedNotifications
{
	AudioHandler::AudioHandler (const AudioThemeManager *mgr)
	: AudioThemeMgr_ { mgr }
	{
	}

	NotificationMethod AudioHandler::GetHandlerMethod () const
	{
		return NMAudio;
	}

	void AudioHandler::Handle (const Entity&, const NotificationRule& rule)
	{
		if (!XmlSettingsManager::Instance ()
				.property ("EnableAudioNots").toBool ())
			return;

		const auto& fname = AudioThemeMgr_->GetAbsoluteFilePath (rule.GetAudioParams ().Filename_);

		const auto& now = QDateTime::currentDateTime ();
		if (LastNotify_ [fname].msecsTo (now) < 1000)
			return;

		LastNotify_ [fname] = now;

		const auto& e = Util::MakeEntity (fname, QString (), Internal);
		GH_->GetProxy ()->GetEntityManager ()->HandleEntity (e);
	}
}
