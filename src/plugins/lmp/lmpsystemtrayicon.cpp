/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "lmpsystemtrayicon.h"
#include <util/gui/fancytrayicon.h>
#include <util/sll/qtutil.h>
#include <util/util.h>
#include "util.h"

namespace LC::LMP
{
	LMPSystemTrayIcon::LMPSystemTrayIcon (const QIcon& icon, QObject *parent)
	: QObject { parent }
	, Icon_ { *new Util::FancyTrayIcon { { .Id_ = QStringLiteral ("LMP"), .Title_ = QStringLiteral ("LeechCraft LMP") }, this } }
	{
		SetIcon (icon);
		UpdateSongInfo ({});

		connect (&Icon_,
				&Util::FancyTrayIcon::secondaryActivated,
				this,
				&LMPSystemTrayIcon::playPauseToggled);

		connect (&Icon_,
				&Util::FancyTrayIcon::scrolled,
				this,
				[this] (int delta, Qt::Orientation orient)
				{
					if (orient == Qt::Vertical)
						emit changedVolume (delta / 120.);
				});
	}

	void LMPSystemTrayIcon::SetMenu (QMenu *menu)
	{
		Icon_.SetContextMenu (menu);
	}

	void LMPSystemTrayIcon::SetVisible (bool visible)
	{
		Icon_.SetVisible (visible);
	}

	void LMPSystemTrayIcon::SetIcon (const QIcon& icon)
	{
		Icon_.SetIcon (icon);
	}

	namespace
	{
		QString MakePlainTooltip (const MediaInfo& song)
		{
			if (song.Title_.isEmpty ())
				return LMPSystemTrayIcon::tr ("No track is currently playing");

			return LMPSystemTrayIcon::tr ("%1 from %2 by %3")
					.arg (song.Title_,
						  song.Album_,
						  song.Artist_);
		}

		auto PrepareSerialized (const QString& name)
		{
			const auto& icon = QIcon::fromTheme (name);
			return Util::GetAsBase64Src (icon.pixmap (24, 24).toImage ());
		}

		QString MakeHtmlTooltip (const MediaInfo& song)
		{
			if (song.Title_.isEmpty ())
				return LMPSystemTrayIcon::tr ("No track is currently playing");

			const auto& albumArt = song.LocalPath_.isEmpty () ?
					QString {} :
					FindAlbumArtPath (song.LocalPath_);

			if (albumArt.isEmpty ())
				return LMPSystemTrayIcon::tr ("%1 from %2 by %3")
						.arg ("<b>" + song.Title_ + "</b>",
							  "<b>" + song.Album_ + "</b>",
							  "<b>" + song.Artist_ + "</b>");

			static const auto artistIcon = PrepareSerialized ("view-media-artist");
			static const auto albumIcon = PrepareSerialized ("media-optical");
			static const auto titleIcon = PrepareSerialized ("media-playback-start");

			return u"<img src='%1' width='%2' height='%2'/><br/><img src='%3'/>%4<br/><img src='%5'/>%6<br/><img src='%7'/>%8"_qsv
					.arg (albumArt,
						  QString::number (400),
						  titleIcon,
						  song.Title_,
						  artistIcon,
						  song.Artist_,
						  albumIcon,
						  song.Album_);
		}
	}

	void LMPSystemTrayIcon::UpdateSongInfo (const MediaInfo& song)
	{
		Icon_.SetToolTip ({
				.PlainText_ = MakePlainTooltip (song),
				.HTML_ = MakeHtmlTooltip (song)
			});
	}
}
