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

#include <QAbstractItemModel>
#include <QFileInfo>
#include <QStringList>
#include <interfaces/lmp/mediainfo.h>

namespace LeechCraft
{
namespace LMP
{
namespace Graffiti
{
	class FilesModel : public QAbstractItemModel
	{
		Q_OBJECT

		const QStringList Headers_;

		struct File
		{
			QString Path_;

			QString Name_;

			MediaInfo Info_;
			MediaInfo OrigInfo_;

			File (const QFileInfo&);
		};
		QList<File> Files_;

		enum Columns
		{
			Title,
			Album,
			Artist,
			Filename,

			MaxColumn
		};
	public:
		enum Roles
		{
			MediaInfoRole = Qt::UserRole + 1,
			OrigMediaInfo
		};

		FilesModel (QObject*);

		QModelIndex index (int, int, const QModelIndex& = QModelIndex ()) const;
		QModelIndex parent (const QModelIndex&) const;
		int rowCount (const QModelIndex&) const;
		int columnCount (const QModelIndex&) const;
		QVariant headerData (int, Qt::Orientation, int) const;
		QVariant data (const QModelIndex&, int) const;

		void AddFiles (const QList<QFileInfo>&);

		void SetInfos (const QList<MediaInfo>&);
		void UpdateInfo (const QModelIndex&, const MediaInfo&);

		void Clear ();

		QList<QPair<MediaInfo, MediaInfo>> GetModified () const;
	private:
		QList<File>::iterator FindFile (const QString&);
	};
}
}
}
