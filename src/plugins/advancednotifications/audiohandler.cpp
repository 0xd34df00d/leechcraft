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

#include "audiohandler.h"
#include <util/util.h>
#include <util/resourceloader.h>
#include "core.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace AdvancedNotifications
{
	AudioHandler::AudioHandler ()
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

		QString fname = rule.GetAudioParams ().Filename_;
		if (fname.isEmpty ())
			return;
		
		if (!fname.contains ('/'))
		{
			const QString& option = XmlSettingsManager::Instance ()
					.property ("AudioTheme").toString ();
			const QString& base = option + '/' + fname;

			QStringList pathVariants;
			pathVariants << base + ".ogg"
					<< base + ".wav"
					<< base + ".flac"
					<< base + ".mp3";

			fname = Core::Instance ().GetAudioThemeLoader ()->GetPath (pathVariants);
		}

		const Entity& e = Util::MakeEntity (fname, QString (), Internal);
		Core::Instance ().SendEntity (e);
	}
}
}
