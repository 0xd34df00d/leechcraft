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

#pragma once

#include <QDialog>
#include <interfaces/media/ialbumartprovider.h>
#include "ui_albumartmanagerdialog.h"

class QStandardItemModel;

namespace LeechCraft
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

		bool RequestScheduled_;
	public:
		AlbumArtManagerDialog (const QString& artist, const QString& album, AlbumArtManager*, QWidget* = 0);

		QString GetArtist () const;
		QString GetAlbum () const;
	public slots:
		void accept ();
	private slots:
		void handleImages (const Media::AlbumInfo&, const QList<QImage>&);
		void handleResized ();
		void request ();
		void requestScheduled ();
		void scheduleRequest ();
	};
}
}
