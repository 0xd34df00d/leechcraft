/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "biopropproxy.h"
#include <algorithm>
#include <QStandardItemModel>
#include <QApplication>
#include <QtDebug>
#include <util/models/rolenamesmixin.h>
#include <util/sll/prelude.h>
#include "xmlsettingsmanager.h"

namespace LC
{
namespace LMP
{
	namespace
	{
		class ArtistImagesModel : public Util::RoleNamesMixin<QStandardItemModel>
		{
		public:
			enum Role
			{
				ThumbURL = Qt::UserRole + 1,
				FullURL,
				Title,
				Author,
				Date
			};

			ArtistImagesModel (QObject *parent)
			: Util::RoleNamesMixin<QStandardItemModel> (parent)
			{
				QHash<int, QByteArray> roleNames;
				roleNames [Role::ThumbURL] = "thumbURL";
				roleNames [Role::FullURL] = "fullURL";
				roleNames [Role::Title] = "title";
				roleNames [Role::Author] = "author";
				roleNames [Role::Date] = "date";
				setRoleNames (roleNames);
			}
		};
	}

	BioPropProxy::BioPropProxy (QObject *parent)
	: QObject (parent)
	, ArtistImages_ (new ArtistImagesModel (this))
	{
	}

	void BioPropProxy::SetBio (const Media::ArtistBio& bio)
	{
		if (Bio_ == bio)
			return;

		Bio_ = bio;

		CachedTags_ = Util::Map (Bio_.BasicInfo_.Tags_, &Media::TagInfo::Name_).join ("; ");

		CachedInfo_ = Bio_.BasicInfo_.FullDesc_.isEmpty () ?
				Bio_.BasicInfo_.ShortDesc_ :
				Bio_.BasicInfo_.FullDesc_;
		CachedInfo_.replace ("\n", "<br />");

		if (auto rc = ArtistImages_->rowCount ())
			ArtistImages_->removeRows (0, rc);

		SetOtherImages (bio.OtherImages_);

		emit artistNameChanged (GetArtistName ());
		emit artistImageURLChanged (GetArtistImageURL ());
		emit artistBigImageURLChanged (GetArtistBigImageURL ());
		emit artistTagsChanged (GetArtistTags ());
		emit artistInfoChanged (GetArtistInfo ());
		emit artistPageURLChanged (GetArtistPageURL ());
	}

	QString BioPropProxy::GetArtistName () const
	{
		return Bio_.BasicInfo_.Name_;
	}

	QUrl BioPropProxy::GetArtistImageURL () const
	{
		return Bio_.BasicInfo_.Image_;
	}

	QUrl BioPropProxy::GetArtistBigImageURL () const
	{
		return Bio_.BasicInfo_.LargeImage_;
	}

	QString BioPropProxy::GetArtistTags () const
	{
		return CachedTags_;
	}

	QString BioPropProxy::GetArtistInfo () const
	{
		return CachedInfo_;
	}

	QUrl BioPropProxy::GetArtistPageURL () const
	{
		return Bio_.BasicInfo_.Page_;
	}

	QObject* BioPropProxy::GetArtistImagesModel () const
	{
		return ArtistImages_;
	}

	void BioPropProxy::SetOtherImages (const QList<Media::ArtistImage>& images)
	{
		if (!XmlSettingsManager::Instance ().property ("FetchArtistBioPhotos").toBool ())
			return;

		QList<QStandardItem*> rows;
		for (const auto& imageItem : images)
		{
			auto item = new QStandardItem ();
			item->setData (imageItem.Thumb_, ArtistImagesModel::Role::ThumbURL);
			item->setData (imageItem.Full_, ArtistImagesModel::Role::FullURL);
			item->setData (imageItem.Title_, ArtistImagesModel::Role::Title);
			item->setData (imageItem.Author_, ArtistImagesModel::Role::Author);
			item->setData (imageItem.Date_, ArtistImagesModel::Role::Date);
			rows << item;
		}
		if (!rows.isEmpty ())
			ArtistImages_->invisibleRootItem ()->appendRows (rows);
	}
}
}
