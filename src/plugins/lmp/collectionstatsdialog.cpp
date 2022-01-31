/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2012  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "collectionstatsdialog.h"
#include <algorithm>
#include <QKeyEvent>
#include "core.h"
#include "localcollection.h"

namespace LC
{
namespace LMP
{
	namespace
	{
		template<typename T>
		QList<T> findTops (const QHash<T, int>& counts, int number)
		{
			auto keys = counts.keys ();
			const auto pred = [&counts] (const T& t1, const T& t2) { return counts [t1] > counts [t2]; };
			if (number >= keys.size ())
			{
				std::sort (keys.begin (), keys.end (), pred);
				return keys;
			}

			const auto mid = keys.begin () + number;
			std::nth_element (keys.begin (), mid, keys.end (), pred);
			std::sort (keys.begin (), mid);
			keys.erase (mid, keys.end ());
			return keys;
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

			auto nameItem = new QTableWidgetItem (name);
			nameItem->setFlags (Qt::ItemIsSelectable | Qt::ItemIsEnabled);
			Ui_.StatsTable_->setItem (rc, 0, nameItem);

			auto valueItem = new QTableWidgetItem (value);
			valueItem->setFlags (Qt::ItemIsSelectable | Qt::ItemIsEnabled);
			Ui_.StatsTable_->setItem (rc, 1, valueItem);

			Ui_.StatsTable_->resizeColumnsToContents ();
		};

		const auto& artists = Core::Instance ().GetLocalCollection ()->GetAllArtists ();
		addValue (tr ("Artists in collection:"), QString::number (artists.size ()));

		QSet<int> albumIds;
		QHash<int, int> year2albums;
		QHash<QString, int> genre2encounters;
		int trackCount = 0;
		for (const auto& artist : artists)
			for (const auto& album : artist.Albums_)
			{
				if (albumIds.contains (album->ID_))
					continue;

				albumIds << album->ID_;
				++year2albums [album->Year_];
				trackCount += album->Tracks_.size ();

				if (!album->Tracks_.isEmpty ())
					for (const auto& genre : album->Tracks_.at (0).Genres_)
						++genre2encounters [genre.toLower ()];
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

	void CollectionStatsDialog::keyReleaseEvent (QKeyEvent *e)
	{
		if (e->key () == Qt::Key_Escape)
		{
			deleteLater ();
			return;
		}
		QWidget::keyReleaseEvent (e);
	}
}
}
