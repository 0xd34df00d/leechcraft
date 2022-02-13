/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include <interfaces/media/idiscographyprovider.h>
#include "ui_releaseswidget.h"

class QStandardItemModel;
class QQuickWidget;

namespace Media
{
	class IRecentReleases;
	struct AlbumRelease;
}

namespace LC
{
namespace LMP
{
	class ReleasesWidget : public QWidget
	{
		Q_OBJECT

		Ui::ReleasesWidget Ui_;

		QQuickWidget * const ReleasesView_;

		QList<Media::IRecentReleases*> Providers_;
		QList<Media::IDiscographyProvider*> DiscoProviders_;

		QStandardItemModel * const ReleasesModel_;

		QVector<QList<Media::ReleaseTrackInfo>> TrackLists_;
	public:
		ReleasesWidget (QWidget* = 0);

		void InitializeProviders ();
	private:
		void HandleRecentReleases (const QList<Media::AlbumRelease>&);
	private slots:
		void request ();
	};
}
}
