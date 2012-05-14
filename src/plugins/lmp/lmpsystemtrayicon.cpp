/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
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

#include "lmpsystemtrayicon.h"
#include <QWheelEvent>
#include <QHelpEvent>
#include <QToolTip>
#include <phonon/audiooutput.h>
#include "core.h"
#include "playertab.h"
#include "util.h"
#include "player.h"

namespace LeechCraft
{
namespace LMP
{
	LMPSystemTrayIcon::LMPSystemTrayIcon (const QIcon& icon, QObject *parent)
	: QSystemTrayIcon (icon, parent)
	, PlayerTab_ (qobject_cast<PlayerTab*> (parent))
	{
	}

	bool LMPSystemTrayIcon::event (QEvent *event)
	{
		if (event->type () == QEvent::Wheel)
		{
			QWheelEvent *wheel = static_cast<QWheelEvent*> (event);
			emit changedVolume (wheel->delta ());

			return true;
		}
		else if (event->type () == QEvent::ToolTip)
		{
			QHelpEvent *help = static_cast<QHelpEvent*> (event);
			QString text;

			if (PlayerTab_ &&
					!CurrentSong_.Title_.isEmpty ())
			{
				const QString& trackText = tr ("%1 (%2)")
						.arg ("<b>" + CurrentSong_.Title_ + "</b>")
						.arg ("<b>" + QTime ().addSecs (CurrentSong_.Length_).toString ("mm:ss") + "</b>");
				auto ao = PlayerTab_->GetPlayer ()->GetAudioOutput ();
				int vol = 0;
				if (ao)
				{
					qreal volume = ao->volume ();
					vol = volume * 100;
				}
				const QString& volumeText = tr ("Volume: %1%")
						.arg (vol);

				text = QString ("<table border='0'>"
						"<tr><td align='center' valign='top' rowspan='5'><img src='%1' width='%2' height='%3'></td></tr>"
						"<tr><td><p style='white-space:pre;'>%4</p></td></tr>"
						"<tr><td><p style='white-space:pre;'>%5</p></td></tr>"
						"<tr><td><p style='white-space:pre;'>%6</p></td></tr>"
						"<tr><td><p style='white-space:pre;'>%7</p></td></tr>"
						"</table>")
						.arg (CurrentAlbumArt_)
						.arg (130)
						.arg (130)
						.arg (trackText)
						.arg ("<b>" + CurrentSong_.Album_ + "</b>")
						.arg ("<b>" + CurrentSong_.Artist_ + "</b>")
						.arg ("<em>" + volumeText + "</em>");
			}
			else if (CurrentSong_.Title_.isEmpty ())
				text = QString ("<table border='0'><tr>"
						"<td align='center' valign='middle'><img src='%1' width='%2' height='%3'></td>"
						"<td align='center' valign='middle'><b>%4</b><br>%5</td>"
						"</tr></table>")
						.arg (":/lmp/resources/images/lmp.svg")
						.arg (48)
						.arg (48)
						.arg ("LMP")
						.arg (tr ("No track playing"));

			QToolTip::showText (help->globalPos (), text);

			return true;
		}

		return QSystemTrayIcon::event (event);
	}

	void LMPSystemTrayIcon::handleSongChanged (const MediaInfo& song)
	{
		CurrentSong_ = song;
		CurrentAlbumArt_ = FindAlbumArtPath (song.LocalPath_);
	}

}
}
