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
#include <interfaces/media/itopprovider.h>
#include "similarmodel.h"
#include "ui_chartswidget.h"

class QQuickWidget;

namespace LC::Util
{
	template<typename T>
	class NamedItemsModel;
}

namespace LC::LMP
{
	class ChartsWidget : public QWidget
	{
		Q_DECLARE_TR_FUNCTIONS (LC::LMP::ChartsWidget)
	public:
		struct TopTrack;
		using TracksModel = Util::NamedItemsModel<TopTrack>;
	private:
		Ui::ChartsWidget Ui_;

		QQuickWidget * const ChartsView_;

		SimilarModel * const TopArtistsModel_;
		TracksModel * const TopTracksModel_;

		QList<QObject*> Providers_;
	public:
		explicit ChartsWidget (QWidget* = nullptr);

		void InitializeProviders ();
	private:
		void HandleArtists (const QList<Media::TopArtistInfo>&);
		void HandleTracks (const QList<Media::TopTrackInfo>&);
		void Request ();
	};
}
