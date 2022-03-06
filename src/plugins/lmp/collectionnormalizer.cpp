/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "collectionnormalizer.h"
#include <QtDebug>
#include <util/sll/prelude.h>

namespace LC::LMP
{
	namespace
	{
		void DumpAlbumsSet (const QList<Collection::Album_ptr>& albumsSet, const char *context)
		{
			if (qgetenv ("LC_LMP_DEBUG_UNITE_SPLITS") != "1")
				return;

			qDebug () << context;
			qDebug () << "\t" << albumsSet [0]->Name_;
			for (const auto& album : albumsSet)
			{
				qDebug () << "\tnext album:";
				for (const auto& track : album->Tracks_)
					qDebug () << "\t\t" << track.Number_ << track.Name_ << track.Length_;
			}
		}

		uint8_t LevDist (const QString& str1, const QString& str2)
		{
			const auto len1 = str1.size ();
			const auto len2 = str2.size ();

			const auto max = std::numeric_limits<uint8_t>::max ();
			if (len1 >= max || len2 >= max)
				return max;

			std::vector<uint8_t> lastColumn (len2 + 1);

			auto prevColumn = lastColumn;
			std::iota (prevColumn.begin (), prevColumn.end (), 0);

			for (int i = 0; i < len1; ++i)
			{
				lastColumn [0] = i + 1;
				for (int j = 0; j < len2; ++j)
					lastColumn [j + 1] = std::min ({
							prevColumn [1 + j] + 1,
							lastColumn [j] + 1,
							prevColumn [j] + (str1 [i] != str2 [j])
					});

				using std::swap;
				swap (lastColumn, prevColumn);
			}

			return prevColumn [len2];
		}

		/* Note that operator== and operator< aren't fully consistent.
		 * Presumably that's OK as operator< is used to sort the list of
		 * tracks, and only then operator== is used to remove duplicates
		 * in that list. operator<'s induced equality is more strict, and
		 * if the proxied strings are compared last, chances are they are
		 * effectively equal from the user's point of view if they are next
		 * to each other in the track list, as all other fields like track
		 * number or length should match for this. In this particular case
		 * it's safe for operator== to induce a more coarse equivalence
		 * relation.
		 */
		struct LevProxy
		{
			const QString& Str_;

			bool operator== (const LevProxy& other) const
			{
				if (Str_ == other.Str_)
					return true;

				return LevDist (Str_, other.Str_) <= 3;
			}

			bool operator< (const LevProxy& other) const
			{
				return Str_ < other.Str_;
			}
		};

		bool UniteSplitTryMerge (Collection::Artists_t& artists, const QList<Collection::Album_ptr>& albumsSet)
		{
			DumpAlbumsSet (albumsSet, "initial state");

			const auto trackPred = [] (int idx)
			{
				return [idx] (const auto& album)
				{
					return std::any_of (album->Tracks_.begin (), album->Tracks_.end (),
							[idx] (const auto& track) { return track.Number_ == idx; });
				};
			};

			const auto firstTrackCounts = std::count_if (albumsSet.begin (), albumsSet.end (), trackPred (1));
			if (firstTrackCounts != 1)
			{
				qDebug () << "cannot break"
						<< firstTrackCounts
						<< "ties for albums"
						<< albumsSet [0]->Name_
						<< albumsSet [0]->Year_
						<< "\n";
				return false;
			}

			for (const auto& album : albumsSet)
			{
				const auto toTuple = [] (const auto& track)
				{ return std::make_tuple (track.Number_, track.Length_, LevProxy { track.Name_ }); };

				std::sort (album->Tracks_.begin (), album->Tracks_.end (), Util::ComparingBy (toTuple));
				const auto unique = std::unique (album->Tracks_.begin (), album->Tracks_.end (),
						Util::EqualityBy (toTuple));
				album->Tracks_.erase (unique, album->Tracks_.end ());
			}

			DumpAlbumsSet (albumsSet, "after unification");

			auto firstPos = std::find_if (albumsSet.begin (), albumsSet.end (), trackPred (1));
			while (true)
			{
				const auto curTracksCount = (*firstPos)->Tracks_.size ();
				auto nextPos = std::find_if (albumsSet.begin (), albumsSet.end (), trackPred (curTracksCount + 1));
				if (nextPos == albumsSet.end ())
					break;

				(*firstPos)->Tracks_ += (*nextPos)->Tracks_;

				for (auto& artist : artists)
				{
					const auto otherIdx = artist.Albums_.indexOf (*nextPos);
					if (otherIdx >= 0)
						artist.Albums_ [otherIdx] = *firstPos;
				}
			}

			DumpAlbumsSet (albumsSet, "after whole processing");

			return true;
		}

		void UniteSplitAlbums (Collection::Artists_t& artists)
		{
			QHash<QPair<int, QString>, QList<Collection::Album_ptr>> potentialSplitAlbums;

			for (const auto& artist : artists)
				for (const auto& album : artist.Albums_)
					potentialSplitAlbums [{ album->Year_, album->Name_ }] << album;

			for (auto it = potentialSplitAlbums.begin (); it != potentialSplitAlbums.end (); )
			{
				if (it.value ().size () == 1)
					it = potentialSplitAlbums.erase (it);
				else
					++it;
			}

			if (potentialSplitAlbums.isEmpty ())
				return;

			qDebug () << Q_FUNC_INFO
					<< "candidates:"
					<< potentialSplitAlbums.keys ();
			for (const auto& albumsSet : potentialSplitAlbums)
				UniteSplitTryMerge (artists, albumsSet);
		}
	}

	void NormalizeArtistsInfos (Collection::Artists_t& artists)
	{
		UniteSplitAlbums (artists);
	}
}
