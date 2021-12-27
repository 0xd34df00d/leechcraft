/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QCoreApplication>
#include <QDialog>
#include <interfaces/media/ialbumartprovider.h>
#include "ui_albumartmanagerdialog.h"

class QStandardItemModel;

namespace LC::LMP
{
	class AlbumArtManager;

	class AlbumArtManagerDialog : public QDialog
	{
		Q_DECLARE_TR_FUNCTIONS (LC::LMP::AlbumArtManagerDialog)

		AlbumArtManager * const AAMgr_;

		Ui::AlbumArtManagerDialog Ui_;

		QStandardItemModel *Model_;
		QList<QImage> FullImages_;

		const int AlbumID_;
		const QString Artist_;
		const QString Album_;

		QTimer * const ReqTimer_;
	public:
		AlbumArtManagerDialog (int albumId,
				const QString& artist,
				const QString& album,
				AlbumArtManager*,
				QWidget* = nullptr);

		QString GetArtist () const;
		QString GetAlbum () const;

		void ScheduleRequest ();
	protected:
		void accept () override;
	private:
		void BrowseImage ();
		void HandleImages (const QList<QImage>&);

		void Request ();
	};
}
