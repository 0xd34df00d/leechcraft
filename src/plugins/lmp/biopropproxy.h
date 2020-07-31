/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/media/iartistbiofetcher.h>

class QStandardItemModel;

namespace Media
{
	struct ArtistImage;
}

namespace LC
{
namespace LMP
{
	class BioPropProxy : public QObject
	{
		Q_OBJECT

		Q_PROPERTY (QString artistName READ GetArtistName NOTIFY artistNameChanged)
		Q_PROPERTY (QUrl artistImageURL READ GetArtistImageURL NOTIFY artistImageURLChanged)
		Q_PROPERTY (QUrl artistBigImageURL READ GetArtistBigImageURL NOTIFY artistBigImageURLChanged)
		Q_PROPERTY (QString artistTags READ GetArtistTags NOTIFY artistTagsChanged)
		Q_PROPERTY (QString artistInfo READ GetArtistInfo NOTIFY artistInfoChanged)
		Q_PROPERTY (QUrl artistPageURL READ GetArtistPageURL NOTIFY artistPageURLChanged)
		Q_PROPERTY (QObject* artistImagesModel READ GetArtistImagesModel NOTIFY artistImagesModelChanged)

		Media::ArtistBio Bio_;

		QString CachedTags_;
		QString CachedInfo_;

		QStandardItemModel *ArtistImages_;
	public:
		BioPropProxy (QObject* = 0);

		void SetBio (const Media::ArtistBio&);

		QString GetArtistName () const;
		QUrl GetArtistImageURL () const;
		QUrl GetArtistBigImageURL () const;
		QString GetArtistTags () const;
		QString GetArtistInfo () const;
		QUrl GetArtistPageURL () const;
		QObject* GetArtistImagesModel () const;
	private:
		void SetOtherImages (const QList<Media::ArtistImage>&);
	signals:
		void artistNameChanged (const QString&);
		void artistImageURLChanged (const QUrl&);
		void artistBigImageURLChanged (const QUrl&);
		void artistTagsChanged (const QString&);
		void artistInfoChanged (const QString&);
		void artistPageURLChanged (const QUrl&);
		void artistImagesModelChanged (QObject*);
	};
}
}
