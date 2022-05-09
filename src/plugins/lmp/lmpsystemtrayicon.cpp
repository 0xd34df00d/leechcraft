/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "lmpsystemtrayicon.h"
#include <QTime>
#include <util/sll/qtutil.h>
#include "util.h"

namespace LC::LMP
{
	LMPSystemTrayIcon::LMPSystemTrayIcon (const QIcon& icon, QObject *parent)
	: QSystemTrayIcon { icon, parent }
	{
		UpdateSongInfo ({});
	}

	namespace
	{
		QString MakeTooltip (const MediaInfo& song)
		{
			if (song.Title_.isEmpty ())
				return u"<table border='0'><tr>"
					   "<td align='center' valign='middle'><img src='%1' width='%2' height='%2'></td>"
					   "<td align='center' valign='middle'><b>%3</b><br>%4</td>"
					   "</tr></table>"_qsv
						.arg ("lcicons:/lmp/resources/images/lmp.svg",
							  QString::number (48),
							  "LMP",
							  LMPSystemTrayIcon::tr ("No track playing"));

			const auto& albumArt = song.LocalPath_.isEmpty () ?
					QString {} :
					FindAlbumArtPath (song.LocalPath_);

			const auto& trackText = QStringLiteral ("<b>%1</b> (<b>%2</b>)")
					.arg (song.Title_)
					.arg (QTime ().addSecs (song.Length_).toString ("mm:ss"));

			return u"<table border='0'>"
					"<tr><td align='center' valign='top' rowspan='5'><img src='%1' width='%2' height='%2'></td></tr>"
					"<tr><td><p style='white-space:pre;'>%3</p></td></tr>"
					"<tr><td><p style='white-space:pre;'><b>%4</b></p></td></tr>"
					"<tr><td><p style='white-space:pre;'><b>%5</b></p></td></tr>"
					"</table>"_qsv
					.arg (albumArt,
						  QString::number (130),
						  trackText,
						  song.Album_,
						  song.Artist_);
		}
	}

	void LMPSystemTrayIcon::UpdateSongInfo (const MediaInfo& song)
	{
		setToolTip (MakeTooltip (song));
	}
}
