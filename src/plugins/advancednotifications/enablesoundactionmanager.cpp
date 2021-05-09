/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "enablesoundactionmanager.h"
#include <QAction>
#include "xmlsettingsmanager.h"

namespace LC::AdvancedNotifications
{
	EnableSoundActionManager::EnableSoundActionManager (QObject *parent)
	: QObject (parent)
	, EnableAction_ (new QAction (tr ("Enable sound notifications"), this))
	{
		EnableAction_->setCheckable (true);
		EnableAction_->setProperty ("ActionIcon", "audio-volume-high");
		EnableAction_->setProperty ("ActionIconOff", "audio-volume-muted");
		EnableAction_->setProperty ("Action/ID", "org.LeechCraft.AdvancedNotifications.EnableSound");

		auto& xsm = XmlSettingsManager::Instance ();
		connect (EnableAction_,
				&QAction::triggered,
				this,
				[&xsm] (bool enable)
				{
					if (enable != xsm.property ("EnableAudioNots").toBool ())
						xsm.setProperty ("EnableAudioNots", enable);
				});

		xsm.RegisterObject ("EnableAudioNots", this,
				[this] (const QVariant& value) { EnableAction_->setChecked (value.toBool ()); });
	}

	QAction* EnableSoundActionManager::GetAction () const
	{
		return EnableAction_;
	}

	QList<QAction*> EnableSoundActionManager::GetActions (ActionsEmbedPlace aep) const
	{
		QList<QAction*> result;

		switch (aep)
		{
		case ActionsEmbedPlace::QuickLaunch:
		case ActionsEmbedPlace::TrayMenu:
			result << EnableAction_;
		default:
			break;
		}

		return result;
	}
}
