/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include <interfaces/media/ilyricsfinder.h>
#include "ui_nowplayingwidget.h"
#include "mediainfo.h"

namespace LC
{
namespace LMP
{
	struct MediaInfo;
	class ArtistsInfoDisplay;

	class BioWidget;
	class SimilarView;

	class NowPlayingWidget : public QWidget
	{
		Q_OBJECT

		Ui::NowPlayingWidget Ui_;
		BioWidget * const BioWidget_;
		SimilarView * const SimilarView_;

		Media::SimilarityInfos_t LastInfos_;
		MediaInfo CurrentInfo_;

		QList<Media::LyricsResultItem> PossibleLyrics_;
		int LyricsVariantPos_ = 0;
	public:
		NowPlayingWidget (QWidget* = nullptr);

		void AddTab (const QString&, QWidget*);

		void SetSimilarArtists (Media::SimilarityInfos_t);
		void SetLyrics (const Media::LyricsResultItem&);

		void SetTrackInfo (const MediaInfo&);
	private slots:
		void on_PrevLyricsButton__released ();
		void on_NextLyricsButton__released ();
		void updateLyricsSwitcher ();
		void on_LyricsBrowser__customContextMenuRequested (const QPoint&);

		void resetSimilarArtists ();
	signals:
		void gotArtistImage (const QString&, const QUrl&);
	};
}
}
