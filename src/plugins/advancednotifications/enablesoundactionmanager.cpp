/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "enablesoundactionmanager.h"
#include <QAction>
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace AdvancedNotifications
{
	EnableSoundActionManager::EnableSoundActionManager (QObject *parent)
	: QObject (parent)
	, EnableAction_ (new QAction (tr ("Enable sound notifications"), this))
	{
		EnableAction_->setCheckable (true);
		EnableAction_->setProperty ("ActionIcon", "preferences-desktop-sound");
		EnableAction_->setProperty ("Action/ID", "org.LeechCraft.AdvancedNotifications.EnableSound");

		connect (EnableAction_,
				SIGNAL (toggled (bool)),
				this,
				SLOT (enableSounds (bool)));

		XmlSettingsManager::Instance ().RegisterObject ("EnableAudioNots",
				this, "xsdPropChanged");
		xsdPropChanged ();
	}

	QList<QAction*> EnableSoundActionManager::GetActions (ActionsEmbedPlace aep) const
	{
		QList<QAction*> result;

		switch (aep)
		{
		case AEPQuickLaunch:
		case AEPTrayMenu:
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
