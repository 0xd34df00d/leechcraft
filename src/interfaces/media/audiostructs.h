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

#include <QStringList>
#include <QVariantMap>
#include <QUrl>

namespace Media
{
	struct AudioInfo
	{
		QString Artist_;
		QString Album_;
		QString Title_;

		QStringList Genres_;

		qint32 Length_;
		qint32 Year_;
		qint32 TrackNumber_;

		/** Other fields known to be used:
		 * - URL with a QUrl pointing to either local file (if the scheme
		 *   is "file:") or a remote file or radio stream otherwise.
		 */
		QVariantMap Other_;
	};

	struct TagInfo
	{
		QString Name_;
	};
	typedef QList<TagInfo> TagInfos_t;

	struct ArtistInfo
	{
		QString Name_;

		QString ShortDesc_;
		QString FullDesc_;

		QUrl Image_;
		QUrl LargeImage_;
		QUrl Page_;

		TagInfos_t Tags_;
	};

	struct SimilarityInfo
	{
		ArtistInfo Artist_;
		int Similarity_;
		QStringList SimilarTo_;
	};
	typedef QList<SimilarityInfo> SimilarityInfos_t;
}

Q_DECLARE_METATYPE (Media::AudioInfo);
Q_DECLARE_METATYPE (QList<Media::AudioInfo>);
