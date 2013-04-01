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
	/** @brief Describes a single audio track.
	 */
	struct AudioInfo
	{
		/** The artist performing this track.
		 */
		QString Artist_;

		/** The album this track is on.
		 */
		QString Album_;

		/** The title of this track.
		 */
		QString Title_;

		/** The genres of this track.
		 */
		QStringList Genres_;

		/** The length of this track in seconds.
		 */
		qint32 Length_;

		/** The year of the Album_ this track is on.
		 */
		qint32 Year_;

		/** The number of this track on the Album_.
		 */
		qint32 TrackNumber_;

		/** Other fields known to be used:
		 * - URL with a QUrl pointing to either local file (if the scheme
		 *   is "file:") or a remote file or radio stream otherwise.
		 */
		QVariantMap Other_;
	};

	/** @brief Information about a tag like a genre.
	 */
	struct TagInfo
	{
		/** Name of the tag.
		 */
		QString Name_;
	};

	/** @brief A list of tags.
	 */
	typedef QList<TagInfo> TagInfos_t;

	/** @brief A structure describing an artist.
	 */
	struct ArtistInfo
	{
		/** The artist name.
		 */
		QString Name_;

		/** Short artist description.
		 */
		QString ShortDesc_;

		/** Full artist description, not including the short description.
		 */
		QString FullDesc_;

		/** An URL of a thumbnail artist image.
		 */
		QUrl Image_;

		/** A bigger artist image.
		 */
		QUrl LargeImage_;

		/** @brief An URL to a page describing this artist.
		 *
		 * Generally this should be set to the artist page on some
		 * service this ArtistInfo is fetched from, not to the artist's
		 * own web site. For example, a Last.FM client would set
		 * this to "http://www.last.fm/music/Fellsilent" for Fellsilent
		 * band.
		 */
		QUrl Page_;

		/** Genres this artist plays in.
		 */
		TagInfos_t Tags_;
	};

	/** @brief Describes similarty information of an artist.
	 *
	 * Similarity information may contain similarity percentage (the
	 * Similarity_ field) with respect to some other artist or artist
	 * names similar to this one. For example, both IRecommendedArtists
	 * and ISimilarArtists return IPendingSimilarArtists which carries
	 * a list of SimilarityInfo structures, but IRecommendedArtists will
	 * typically contain the list of names of artists from the user
	 * library that an artist is similar to, while ISimilarArtists will
	 * carry around similarity percentage.
	 *
	 * @sa ISimilarArtists, IRecommendedArtists, IPendingSimilarArtists
	 */
	struct SimilarityInfo
	{
		/** Information about artist this similary info is about.
		 */
		ArtistInfo Artist_;

		/** @brief Similarity in percents.
		 *
		 * May be 0 if only SimilarTo_ field is filled.
		 */
		int Similarity_;

		/** Names of the artists similar to this one.
		 */
		QStringList SimilarTo_;
	};
	typedef QList<SimilarityInfo> SimilarityInfos_t;
}

Q_DECLARE_METATYPE (Media::AudioInfo);
Q_DECLARE_METATYPE (QList<Media::AudioInfo>);
