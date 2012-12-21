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
#include "audiostructs.h"

class QObject;

namespace Media
{
	class IPendingAudioSearch
	{
	public:
		virtual ~IPendingAudioSearch () {}

		virtual QObject* GetObject () = 0;

		struct Result
		{
			AudioInfo Info_;
			QUrl Source_;
		};

		virtual QList<Result> GetResults () const = 0;
	protected:
		virtual void ready () = 0;
	};

	struct AudioSearchRequest
	{
		QString Title_;
		QString Artist_;
		QString Album_;

		int TrackLength_;

		QString FreeForm_;

		AudioSearchRequest ()
		: TrackLength_ (0)
		{
		}
	};

	class IAudioPile
	{
	public:
		virtual ~IAudioPile () {}

		virtual IPendingAudioSearch* Search (const AudioSearchRequest&) = 0;
	};
}

Q_DECLARE_INTERFACE (Media::IPendingAudioSearch, "org.LeechCraft.Media.IPendingAudioSearch/1.0");
Q_DECLARE_INTERFACE (Media::IAudioPile, "org.LeechCraft.Media.IAudioPile/1.0");
