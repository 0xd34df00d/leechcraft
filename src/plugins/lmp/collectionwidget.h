/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include <interfaces/core/ihookproxy.h>
#include "ui_collectionwidget.h"

class QSortFilterProxyModel;

namespace LC::LMP
{
	class Player;
	struct MediaInfo;

	class CollectionWidget : public QWidget
	{
		Q_OBJECT

		Ui::CollectionWidget Ui_;

		Player * const Player_;

		QSortFilterProxyModel * const CollectionFilterModel_;
	public:
		CollectionWidget (QWidget* = nullptr);
	private:
		void ShowCollectionTrackProps ();
		void ShowCollectionAlbumArt ();
		void ShowAlbumArtManager ();
		void ShowInArtistBrowser ();
		void HandleCollectionRemove ();
		void HandleCollectionDelete ();
		void LoadFromCollection ();
		void HandleScanProgress (int);

		void ShowContextMenu (QPoint);
	signals:
		void hookCollectionContextMenuRequested (LC::IHookProxy_ptr,
				QMenu*,
				const LC::LMP::MediaInfo&);
	};
}
