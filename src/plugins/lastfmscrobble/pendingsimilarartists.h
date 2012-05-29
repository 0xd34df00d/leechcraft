/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011 Minh Ngo
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

#include <QObject>
#include <interfaces/media/isimilarartists.h>
#include <interfaces/media/ipendingsimilarartists.h>

namespace LeechCraft
{
namespace Lastfmscrobble
{
	class PendingSimilarArtists : public QObject
								 , public Media::IPendingSimilarArtists
	{
		Q_OBJECT
		Q_INTERFACES (Media::IPendingSimilarArtists)

		const QString SourceName_;
		int NumGet_;
		Media::SimilarityInfos_t Similar_;
		int InfosWaiting_;
	public:
		PendingSimilarArtists (const QString&, int num, QObject* = 0);

		QObject* GetObject ();
		QString GetSourceArtistName () const;
		Media::SimilarityInfos_t GetSimilar () const;
	private:
		void DecrementWaiting ();
	private slots:
		void handleReplyFinished ();
		void handleReplyError ();

		void handleInfoReplyFinished ();
		void handleInfoReplyError ();

		void handleTagsReplyFinished ();
		void handleTagsReplyError ();
	signals:
		void ready ();
		void error ();
	};
}
}
