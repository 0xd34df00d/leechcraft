/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QCoreApplication>
#include <QWidget>
#include <interfaces/media/ihypesprovider.h>
#include "similarmodel.h"
#include "ui_hypeswidget.h"

class QStandardItemModel;
class QQuickWidget;

namespace LC::Util
{
	template<typename T>
	class RoledItemsModel;
}

namespace LC::LMP
{
	class HypesWidget : public QWidget
	{
		Q_DECLARE_TR_FUNCTIONS (LC::LMP::HypesWidget)
	public:
		struct HypedTrack;
		using TracksModel = Util::RoledItemsModel<HypedTrack>;
	private:
		Ui::HypesWidget Ui_;

		QQuickWidget * const HypesView_;

		SimilarModel * const NewArtistsModel_;
		SimilarModel * const TopArtistsModel_;
		TracksModel * const NewTracksModel_;
		TracksModel * const TopTracksModel_;

		QList<QObject*> Providers_;
	public:
		explicit HypesWidget (QWidget* = nullptr);

		void InitializeProviders ();
	private:
		void HandleArtists (const QList<Media::HypedArtistInfo>&, Media::IHypesProvider::HypeType);
		void HandleTracks (const QList<Media::HypedTrackInfo>&, Media::IHypesProvider::HypeType);
		void Request ();
	};
}
