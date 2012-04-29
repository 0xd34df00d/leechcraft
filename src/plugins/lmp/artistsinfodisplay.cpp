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

#include "artistsinfodisplay.h"
#include <algorithm>
#include <QStandardItemModel>
#include <QDeclarativeContext>

namespace LeechCraft
{
namespace LMP
{
	namespace
	{
		class SimilarModel : public QStandardItemModel
		{
		public:
			enum Role
			{
				ArtistName = Qt::UserRole + 1,
				Similarity,
				ArtistImageURL,
				ArtistBigImageURL,
				ArtistPageURL,
				ArtistTags,
				ShortDesc,
				FullDesc
			};

			SimilarModel (QObject *parent = 0)
			: QStandardItemModel (parent)
			{
				QHash<int, QByteArray> names;
				names [ArtistName] = "artistName";
				names [Similarity] = "similarity";
				names [ArtistImageURL] = "artistImageURL";
				names [ArtistBigImageURL] = "artistBigImageURL";
				names [ArtistPageURL] = "artistPageURL";
				names [ArtistTags] = "artistTags";
				names [ShortDesc] = "shortDesc";
				names [FullDesc] = "fullDesc";
				setRoleNames (names);
			}
		};
	}

	ArtistsInfoDisplay::ArtistsInfoDisplay (QWidget *parent)
	: QDeclarativeView (parent)
	, Model_ (new SimilarModel (this))
	{
		rootContext ()->setContextProperty ("similarModel", Model_);
		setSource (QUrl ("qrc:/lmp/resources/qml/SimilarView.qml"));
	}

	void ArtistsInfoDisplay::SetSimilarArtists (Media::SimilarityInfos_t infos)
	{
		Model_->clear ();

		std::sort (infos.begin (), infos.end (),
				[] (const Media::SimilarityInfo_t& left, const Media::SimilarityInfo_t& right)
					{ return left.second > right.second; });

		Q_FOREACH (const Media::SimilarityInfo_t& info, infos)
		{
			auto item = new QStandardItem ();

			const auto& artist = info.first;
			item->setData (artist.Name_, SimilarModel::Role::ArtistName);
			item->setData (artist.Image_, SimilarModel::Role::ArtistImageURL);
			item->setData (artist.ShortDesc_, SimilarModel::Role::ShortDesc);
			item->setData (artist.FullDesc_, SimilarModel::Role::FullDesc);
			item->setData (tr ("Similarity: %1%").arg (info.second), SimilarModel::Role::Similarity);

			QStringList tags;
			const int diff = artist.Tags_.size () - 5;
			auto begin = artist.Tags_.begin ();
			if (diff > 0)
				std::advance (begin, diff);
			std::transform (begin, artist.Tags_.end (), std::back_inserter (tags),
					[] (decltype (artist.Tags_.front ()) tag) { return tag.Name_; });
			std::reverse (tags.begin (), tags.end ());
			item->setData (tr ("Tags: %1").arg (tags.join ("; ")), SimilarModel::Role::ArtistTags);

			Model_->appendRow (item);
		}
	}
}
}
