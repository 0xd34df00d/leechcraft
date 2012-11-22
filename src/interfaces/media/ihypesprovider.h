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

#pragma once

#include <QString>
#include <QList>
#include <QUrl>
#include "audiostructs.h"

namespace Media
{
	struct HypedArtistInfo
	{
		ArtistInfo Info_;

		int PercentageChange_;
		int Playcount_;
		int Listeners_;
	};

	struct HypedTrackInfo
	{
		QString TrackName_;
		QUrl TrackPage_;

		int PercentageChange_;
		int Playcount_;
		int Listeners_;

		int Duration_;

		QUrl Image_;
		QUrl LargeImage_;

		QString ArtistName_;
		QUrl ArtistPage_;
	};

	class IHypesProvider
	{
	public:
		virtual ~IHypesProvider () {}

		virtual QString GetServiceName () const = 0;

		enum class HypeType
		{
			NewArtists,
			NewTracks,
			TopArtists,
			TopTracks
		};

		virtual bool SupportsHype (HypeType) = 0;

		virtual void RequestHype (HypeType) = 0;
	protected:
		virtual void gotHypedArtists (const QList<HypedArtistInfo>&, HypeType) = 0;
		virtual void gotHypedTracks (const QList<HypedTrackInfo>&, HypeType) = 0;
	};
}

Q_DECLARE_INTERFACE (Media::IHypesProvider, "org.LeechCraft.Media.IHypesProvider/1.0");
