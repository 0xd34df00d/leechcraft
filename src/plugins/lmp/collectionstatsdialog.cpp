/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2012  Georg Rudoy
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

#include "collectionstatsdialog.h"
#include <algorithm>
#include "core.h"
#include "localcollection.h"

namespace LeechCraft
{
namespace LMP
{
	namespace
	{
		template<typename T>
		QList<T> findTops (const QHash<T, int>& counts, int number)
		{
			auto keys = counts.keys ();
			std::sort (keys.begin (), keys.end (),
					[&counts] (const T& t1, const T& t2)
						{ return counts [t1] > counts [t2]; });
			return keys.mid (0, number);
		}
	}

	CollectionStatsDialog::CollectionStatsDialog (QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);
		Ui_.StatsTable_->setColumnCount (2);

		auto addValue = [this] (const QString& name, const QString& value)
		{
			const int rc = Ui_.StatsTable_->rowCount ();
			Ui_.StatsTable_->setRowCount (rc + 1);
			Ui_.StatsTable_->setItem (rc, 0, new QTableWidgetItem (name));
			Ui_.StatsTable_->setItem (rc, 1, new QTableWidgetItem (value));
			Ui_.StatsTable_->resizeColumnsToContents ();
		};

		const auto& artists = Core::Instance ().GetLocalCollection ()->GetAllArtists ();
		addValue (tr ("Artists in collection:"), QString::number (artists.size ()));

		QSet<int> albumIds;
		QHash<int, int> year2albums;
		QHash<QString, int> genre2encounters;
		int trackCount = 0;
		Q_FOREACH (const auto& artist, artists)
			Q_FOREACH (const auto& album, artist.Albums_)
			{
				if (albumIds.contains (album->ID_))
					continue;

				albumIds << album->ID_;
				++year2albums [album->Year_];
				trackCount += album->Tracks_.size ();

				if (!album->Tracks_.isEmpty ())
					Q_FOREACH (const auto& genre, album->Tracks_.at (0).Genres_)
						++genre2encounters [genre];
			}

		const int albumsCount = albumIds.size ();
		addValue (tr ("Albums in collection:"), QString::number (albumsCount));
		addValue (tr ("Tracks in collection:"), QString::number (trackCount));
		if (!albumsCount)
			return;

		addValue (tr ("Average tracks per album:"),
				QString::number (static_cast<double> (trackCount) / albumsCount, 'g', 2));

		const auto& years = findTops (year2albums, 10);
		QStringList yearsStrs;
		std::transform (years.begin (), years.end (), std::back_inserter (yearsStrs),
				[] (int year) { return QString::number (year); });
		addValue (tr ("Top 10 album years:"), yearsStrs.join ("; "));

		const auto& genres = QStringList (findTops (genre2encounters, 5));
		addValue (tr ("Top 5 genres:"), genres.join ("; "));
	}
}
}
