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

namespace LC
{
namespace AdvancedNotifications
{
	EnableSoundActionManager::EnableSoundActionManager (QObject *parent)
	: QObject (parent)
	, EnableAction_ (new QAction (tr ("Enable sound notifications"), this))
	{
		EnableAction_->setCheckable (true);
		EnableAction_->setProperty ("ActionIcon", "audio-volume-high");
		EnableAction_->setProperty ("ActionIconOff", "audio-volume-muted");
		EnableAction_->setProperty ("Action/ID", "org.LeechCraft.AdvancedNotifications.EnableSound");

		connect (EnableAction_,
				SIGNAL (toggled (bool)),
				this,
				SLOT (enableSounds (bool)));

		XmlSettingsManager::Instance ().RegisterObject ("EnableAudioNots",
				this, "xsdPropChanged");
		xsdPropChanged ();
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

	void EnableSoundActionManager::xsdPropChanged ()
	{
		EnableAction_->setChecked (XmlSettingsManager::Instance ()
				.property ("EnableAudioNots").toBool ());
	}

	void EnableSoundActionManager::enableSounds (bool enable)
	{
		if (enable != XmlSettingsManager::Instance ()
				.property ("EnableAudioNots").toBool ())
			XmlSettingsManager::Instance ().setProperty ("EnableAudioNots", enable);
	}
}
}
