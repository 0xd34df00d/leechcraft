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

#include "mediainfo.h"

namespace LeechCraft
{
namespace LMP
{
	MediaInfo& MediaInfo::operator= (const Media::AudioInfo& info)
	{
		Artist_ = info.Artist_;
		Album_ = info.Album_;
		Title_ = info.Title_;
		Genres_ = info.Genres_;
		Length_ = info.Length_;
		Year_ = info.Year_;
		TrackNumber_ = info.TrackNumber_;

		return *this;
	}

	MediaInfo::operator Media::AudioInfo () const
	{
		const Media::AudioInfo aInfo =
		{
			Artist_,
			Album_,
			Title_,
			Genres_,
			Length_,
			Year_,
			TrackNumber_,
			QVariantMap ()
		};
		return aInfo;
	}

	MediaInfo MediaInfo::FromAudioInfo (const Media::AudioInfo& info)
	{
		MediaInfo result;
		result = info;
		return result;
	}
}
}