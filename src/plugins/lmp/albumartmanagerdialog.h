/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include <interfaces/media/ialbumartprovider.h>
#include "ui_albumartmanagerdialog.h"

class QStandardItemModel;

namespace LC
{
namespace LMP
{
	class AlbumArtManager;

	class AlbumArtManagerDialog : public QDialog
	{
		Q_OBJECT

		AlbumArtManager * const AAMgr_;

		Ui::AlbumArtManagerDialog Ui_;

		QStandardItemModel *Model_;
		QList<QImage> FullImages_;

		const Media::AlbumInfo SourceInfo_;

		bool RequestScheduled_ = false;
	public:
		AlbumArtManagerDialog (const QString& artist, const QString& album, AlbumArtManager*, QWidget* = 0);

		QString GetArtist () const;
		QString GetAlbum () const;
	public slots:
		void accept ();
	private slots:
		void on_BrowseButton__released ();
		void handleImages (const Media::AlbumInfo&, const QList<QImage>&);
		void handleResized ();
		void request ();
		void requestScheduled ();
		void scheduleRequest ();
	};
}
}
