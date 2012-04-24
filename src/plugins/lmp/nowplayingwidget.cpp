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
#include "mediainfo.h"

namespace LeechCraft
{
namespace LMP
{
	NowPlayingWidget::NowPlayingWidget (QWidget *parent)
	: QWidget (parent)
	{
		Ui_.setupUi (this);
	}

	void NowPlayingWidget::SetAlbumArt (const QPixmap& px)
	{
		Ui_.Art_->setPixmap (px);
	}

	void NowPlayingWidget::SetTrackInfo (const MediaInfo& info)
	{
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
		Ui_.AlbumName_->setText (str (info.Album_));
		Ui_.TrackName_->setText (str (info.Title_));

		const auto& genres = info.Genres_.join (" / ");
		Ui_.Genres_->setText ("<em>" + genres + "</em>");
	}
}
}