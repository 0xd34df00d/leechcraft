/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#include "lcsource.h"
#include <interfaces/media/audiostructs.h>

namespace LeechCraft
{
namespace Azoth
{
namespace Xtazy
{
	LCSource::LCSource (QObject *parent)
	: TuneSourceBase (parent)
	{
		setObjectName ("LCSource");
	}

	void LCSource::NowPlaying (const Media::AudioInfo& audio)
	{
		TuneInfo_t tune;
		tune ["title"] = audio.Title_;
		tune ["artist"] = audio.Artist_;
		tune ["source"] = audio.Album_;
		tune ["track"] = audio.TrackNumber_;
		tune ["length"] = audio.Length_;
		tune ["URL"] = audio.Other_ ["URL"];
		emit tuneInfoChanged (tune);
	}

	void LCSource::Stopped ()
	{
		emit tuneInfoChanged (TuneInfo_t ());
	}
}
}
}
