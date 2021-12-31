/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <optional>
#include <QObject>
#include <QImage>
#include <interfaces/media/idiscographyprovider.h>

class QQuickWidget;

class QStandardItemModel;
class QStandardItem;

namespace Media
{
	class IArtistBioFetcher;
	class IAlbumArtProvider;
	struct AlbumInfo;
}

namespace LC::Util
{
	template<typename>
	class RoledItemsModel;
}

namespace LC
{
namespace LMP
{
	class BioPropProxy;

	class BioViewManager : public QObject
	{
		Q_OBJECT

		QQuickWidget * const View_;

		QString CurrentArtist_;

		BioPropProxy *BioPropProxy_;

		struct DiscoItem;
		using DiscoModel = Util::RoledItemsModel<DiscoItem>;
		DiscoModel * const DiscoModel_;

		QList<QList<Media::ReleaseTrackInfo>> Album2Tracks_;
	public:
		BioViewManager (QQuickWidget*, QObject* = nullptr);

		void InitWithSource ();
		void Request (Media::IArtistBioFetcher*, const QString&, const QStringList&);
	private:
		std::optional<int> FindAlbumItem (const QString&) const;

		bool QueryReleaseImageLocal (const Media::AlbumInfo&) const;
		void QueryReleaseImage (Media::IAlbumArtProvider*, const Media::AlbumInfo&);
		void SetAlbumImage (const QString&, const QUrl&) const;
		void HandleDiscographyReady (QList<Media::ReleaseInfo>);
	private slots:
		void handleAlbumPreviewRequested (int);
	signals:
		void gotArtistImage (const QString&, const QUrl&);
	};
}
}
