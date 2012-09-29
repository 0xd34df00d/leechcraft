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

#include "icecastfetcher.h"
#include "roles.h"
#include <algorithm>
#include <QStandardItem>
#include <QFileInfo>
#include <QUrl>
#include <QDateTime>
#include <QDomDocument>
#include <QTimer>
#include <QtConcurrentRun>
#include <QFutureWatcher>
#include <util/util.h>
#include <interfaces/idownload.h>

Q_DECLARE_METATYPE (QList<QUrl>);

namespace LeechCraft
{
namespace HotStreams
{
	const QString XiphFilename ("yp.xml");

	namespace
	{
		QString GetFilePath ()
		{
			return Util::CreateIfNotExists ("hotstreams/cache").filePath (XiphFilename);
		}

		bool ShouldUpdateFile (const QString& path)
		{
			return QFileInfo (path).lastModified ().daysTo (QDateTime::currentDateTime ()) > 2;
		}
	}

	IcecastFetcher::IcecastFetcher (QStandardItem *root, QNetworkAccessManager*, QObject *parent)
	: QObject (parent)
	, Root_ (root)
	, JobID_ (0)
	, RadioIcon_ (":/hotstreams/resources/images/radio.png")
	{
		auto dir = Util::CreateIfNotExists ("hotstreams/cache");
		const bool exists = dir.exists (XiphFilename);
		if (!exists || ShouldUpdateFile (dir.filePath (XiphFilename)))
		{
			if (exists)
				dir.remove (XiphFilename);

			QTimer::singleShot (0,
					this,
					SLOT (handleFetchList ()));
		}
		else
			ParseList ();
	}

	void IcecastFetcher::FetchList ()
	{
		auto entity = Util::MakeEntity (QUrl ("http://dir.xiph.org/yp.xml"),
				GetFilePath (),
				OnlyDownload |
					Internal |
					DoNotAnnounceEntity |
					DoNotNotifyUser |
					DoNotSaveInHistory);
		QObject *obj = 0;
		emit delegateEntity (entity, &JobID_, &obj);
		if (!obj)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to delegate entity";
			deleteLater ();
			return;
		}

		connect (obj,
				SIGNAL (jobFinished (int)),
				this,
				SLOT (handleJobFinished (int)));
		connect (obj,
				SIGNAL (jobRemoved (int)),
				this,
				SLOT (checkDelete (int)));
	}

	namespace
	{
		struct StationInfo
		{
			QString Name_;
			QString Genre_;
			int Bitrate_;
			QList<QUrl> URLs_;
			QString MIME_;
		};

		typedef QMap<QString, QList<StationInfo>> Stations_t;

		void SortInfoList (QList<StationInfo>& infos)
		{
			std::sort (infos.begin (), infos.end (),
				[] (decltype (infos.at (0)) left, decltype (infos.at (0)) right)
					{ return QString::localeAwareCompare (left.Name_, right.Name_) < 0; });
		}

		Stations_t ParseWorker ()
		{
			QFile file (GetFilePath ());
			if (!file.open (QIODevice::ReadOnly))
			{
				qWarning () << Q_FUNC_INFO
						<< "unable to open file";
				return Stations_t ();
			}

			QDomDocument doc;
			if (!doc.setContent (&file))
			{
				qWarning () << Q_FUNC_INFO
						<< "parse failure, removing the file";
				file.remove ();
				return Stations_t ();
			}

			Stations_t stations;

			auto entry = doc.documentElement ().firstChildElement ("entry");
			while (!entry.isNull ())
			{
				auto getText = [&entry] (const QString& tagName)
				{
					return entry.firstChildElement (tagName).text ();
				};

				const auto& genre = getText ("genre");

				auto& genreStations = stations [genre];
				const StationInfo info
				{
					getText ("server_name"),
					genre,
					getText ("bitrate").toInt (),
					QList<QUrl> () << QUrl (getText ("listen_url")),
					getText ("server_type")
				};
				const auto pos = std::find_if (genreStations.begin (), genreStations.end (),
						[&info] (const StationInfo& otherInfo)
							{ return info.Name_ == otherInfo.Name_ &&
									info.Bitrate_ == otherInfo.Bitrate_ &&
									info.MIME_ == otherInfo.MIME_; });
				if (pos == genreStations.end ())
					genreStations << info;
				else
					pos->URLs_ << info.URLs_;

				entry = entry.nextSiblingElement ("entry");
			}

			if (stations.size () <= 20)
				return stations;

			QList<int> lengths;
			for (const auto& genre : stations.keys ())
				lengths << stations [genre].size ();

			std::sort (lengths.begin (), lengths.end (), std::greater<int> ());
			const int threshold = lengths.at (20);

			QList<StationInfo> otherInfos;
			for (const auto& genre : stations.keys ())
			{
				auto& genreStations = stations [genre];
				if (genreStations.size () <= threshold)
				{
					otherInfos += genreStations;
					stations.remove (genre);
				}
				else
					SortInfoList (genreStations);
			}
			SortInfoList (otherInfos);
			stations ["Other"] = otherInfos;
			return stations;
		}
	}

	void IcecastFetcher::ParseList ()
	{
		auto watcher = new QFutureWatcher<Stations_t> (this);
		connect (watcher,
				SIGNAL (finished ()),
				this,
				SLOT (handleParsed ()));
		watcher->setFuture (QtConcurrent::run (ParseWorker));
	}

	void IcecastFetcher::handleFetchList ()
	{
		FetchList ();
	}

	void IcecastFetcher::handleParsed ()
	{
		auto watcher = dynamic_cast<QFutureWatcher<Stations_t>*> (sender ());
		watcher->deleteLater ();

		const auto& result = watcher->result ();
		for (const auto& genre : result.keys ())
		{
			auto uppercased = genre;
			uppercased [0] = uppercased.at (0).toUpper ();

			auto genreItem = new QStandardItem (uppercased);
			genreItem->setEditable (false);

			for (const auto& station : result [genre])
			{
				const auto& tooltip = tr ("Genre: %1\nBitrate: %2 kbps\nType: %3")
						.arg (station.Genre_)
						.arg (station.Bitrate_)
						.arg (station.MIME_);
				auto item = new QStandardItem (station.Name_);
				item->setToolTip (tooltip);
				item->setIcon (RadioIcon_);
				item->setData (station.Name_, StreamItemRoles::PristineName);
				item->setData (Media::RadioType::Predefined, Media::RadioItemRole::ItemType);
				item->setData ("urllist", StreamItemRoles::PlaylistFormat);
				item->setData (QVariant::fromValue (station.URLs_), Media::RadioItemRole::RadioID);
				item->setEditable (false);

				genreItem->appendRow (item);
			}

			Root_->appendRow (genreItem);
		}
	}

	void IcecastFetcher::handleJobFinished (int id)
	{
		if (id != JobID_)
			return;

		ParseList ();

		checkDelete (id);
	}

	void IcecastFetcher::checkDelete (int id)
	{
		if (id == JobID_)
			deleteLater ();
	}
}
}
