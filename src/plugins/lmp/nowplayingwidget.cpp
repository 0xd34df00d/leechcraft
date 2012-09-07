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

#include "nowplayingwidget.h"
#include <algorithm>
#include <QMouseEvent>
#include "mediainfo.h"
#include "core.h"
#include "localcollection.h"
#include "aalabeleventfilter.h"
#include "util.h"

namespace LeechCraft
{
namespace LMP
{
	NowPlayingWidget::NowPlayingWidget (QWidget *parent)
	: QWidget (parent)
	, LyricsVariantPos_ (0)
	{
		Ui_.setupUi (this);
		connect (Ui_.SimilarIncludeCollection_,
				SIGNAL (stateChanged (int)),
				this,
				SLOT (resetSimilarArtists ()));

		auto coverGetter = [this] () { return CurrentInfo_.LocalPath_; };
		Ui_.Art_->installEventFilter (new AALabelEventFilter (coverGetter, this));

		Ui_.PrevLyricsButton_->setIcon (Core::Instance ().GetProxy ()->GetIcon ("go-previous"));
		Ui_.NextLyricsButton_->setIcon (Core::Instance ().GetProxy ()->GetIcon ("go-next"));

		updateLyricsSwitcher ();

		connect (Ui_.BioWidget_,
				SIGNAL (gotArtistImage (QString, QUrl)),
				this,
				SIGNAL (gotArtistImage (QString, QUrl)));
	}

	void NowPlayingWidget::SetSimilarArtists (Media::SimilarityInfos_t infos)
	{
		LastInfos_ = infos;

		if (Ui_.SimilarIncludeCollection_->checkState () != Qt::Checked)
		{
			auto col = Core::Instance ().GetLocalCollection ();
			auto pos = std::remove_if (infos.begin (), infos.end (),
					[col] (decltype (infos.front ()) item)
						{ return col->FindArtist (item.Artist_.Name_) >= 0; });
			infos.erase (pos, infos.end ());
		}

		Ui_.SimilarView_->SetSimilarArtists (infos);
		Ui_.SimilarView_->setVisible (!infos.isEmpty ());
	}

	void NowPlayingWidget::SetLyrics (const QString& lyrics)
	{
		if (lyrics.simplified ().isEmpty ())
			return;

		if (PossibleLyrics_.contains (lyrics))
			return;

		if (Ui_.LyricsBrowser_->toPlainText ().isEmpty ())
			Ui_.LyricsBrowser_->setHtml (lyrics);

		PossibleLyrics_ << lyrics;
		updateLyricsSwitcher ();
	}

	void NowPlayingWidget::SetAlbumArt (const QPixmap& px)
	{
		Ui_.Art_->setPixmap (px);
	}

	void NowPlayingWidget::SetTrackInfo (const MediaInfo& info)
	{
		CurrentInfo_ = info;

		const bool isNull = info.Title_.isEmpty () && info.Artist_.isEmpty ();
		Ui_.TrackInfoLayout_->setEnabled (!isNull);

		const QString& unknown = isNull ?
				QString () :
				tr ("unknown");
		auto str = [&unknown] (const QString& str)
		{
			return str.isNull () ?
					unknown :
					("<strong>") + str + ("</strong>");
		};
		Ui_.ArtistName_->setText (str (info.Artist_));
		Ui_.AlbumName_->setText (str (fontMetrics ().elidedText (info.Album_, Qt::ElideRight, 300)));
		Ui_.TrackName_->setText (str (info.Title_));

		const auto& genres = info.Genres_.join (" / ");
		Ui_.Genres_->setText ("<em>" + genres + "</em>");

		SetStatistics (info.LocalPath_);

		Ui_.BioWidget_->SetCurrentArtist (info.Artist_);

		Ui_.AudioProps_->SetProps (info);

		PossibleLyrics_.clear ();
		Ui_.LyricsBrowser_->clear ();
		LyricsVariantPos_ = 0;
		updateLyricsSwitcher ();
	}

	namespace
	{
		QString FormatDateTime (const QDateTime& datetime)
		{
			const QDateTime& current = QDateTime::currentDateTime ();
			const int days = datetime.daysTo (current);
			if (days > 30)
				return datetime.toString ("MMMM yyyy");
			else if (days >= 7)
				return NowPlayingWidget::tr ("%n day(s) ago", 0, days);
			else if (days >= 1)
				return datetime.toString ("dddd");
			else
				return datetime.time ().toString ();
		}
	}

	void NowPlayingWidget::SetStatistics (const QString& path)
	{
		auto stats = Core::Instance ().GetLocalCollection ()->GetTrackStats (path);
		const bool valid = stats.Added_.isValid ();
		Ui_.LastPlay_->setVisible (valid);
		Ui_.LabelLastPlay_->setVisible (valid);
		Ui_.StatsCount_->setVisible (valid);
		Ui_.LabelPlaybacks_->setVisible (valid);
		if (!valid)
			return;

		Ui_.LastPlay_->setText (FormatDateTime (stats.LastPlay_));
		Ui_.StatsCount_->setText (tr ("%n play(s) since %1", 0, stats.Playcount_)
					.arg (FormatDateTime (stats.Added_)));
	}

	void NowPlayingWidget::on_PrevLyricsButton__released ()
	{
		if (LyricsVariantPos_ <= 0)
			return;

		Ui_.LyricsBrowser_->setHtml (PossibleLyrics_.at (--LyricsVariantPos_));
		updateLyricsSwitcher ();
	}

	void NowPlayingWidget::on_NextLyricsButton__released ()
	{
		if (LyricsVariantPos_ >= PossibleLyrics_.size () - 1)
			return;

		Ui_.LyricsBrowser_->setHtml (PossibleLyrics_.at (++LyricsVariantPos_));
		updateLyricsSwitcher ();
	}

	void NowPlayingWidget::updateLyricsSwitcher ()
	{
		const auto& size = PossibleLyrics_.size ();
		qDebug () << Q_FUNC_INFO << size << LyricsVariantPos_ << PossibleLyrics_;
		const auto& str = size ?
				tr ("%n possible lyrics found", 0, size) :
				QString ();
		Ui_.LyricsCounter_->setText (str);

		Ui_.PrevLyricsButton_->setEnabled (LyricsVariantPos_);
		Ui_.NextLyricsButton_->setEnabled (LyricsVariantPos_ < size - 1);
	}

	void NowPlayingWidget::resetSimilarArtists ()
	{
		SetSimilarArtists (LastInfos_);
	}
}
}
