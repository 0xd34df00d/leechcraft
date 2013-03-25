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

#pragma once

#include "audiostructs.h"

class QObject;

namespace Media
{
	struct ArtistBio
	{
		ArtistInfo BasicInfo_;
	};

	class Q_DECL_EXPORT IPendingArtistBio
	{
	public:
		virtual ~IPendingArtistBio () {}

		virtual QObject* GetQObject () = 0;

		virtual ArtistBio GetArtistBio () const = 0;
	protected:
		virtual void ready () = 0;
		virtual void error () = 0;
	};

	class Q_DECL_EXPORT IArtistBioFetcher
	{
	public:
		virtual ~IArtistBioFetcher () {}

		virtual QString GetServiceName () const = 0;

		virtual IPendingArtistBio* RequestArtistBio (const QString& artist) = 0;
	};
}

Q_DECLARE_INTERFACE (Media::IPendingArtistBio, "org.LeechCraft.Media.IPendingArtistBio/1.0");
Q_DECLARE_INTERFACE (Media::IArtistBioFetcher, "org.LeechCraft.Media.IArtistBioFetcher/1.0");
