/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QAbstractItemModel>
#include <QFileInfo>
#include <QStringList>
#include <interfaces/lmp/mediainfo.h>

namespace LC
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
			bool IsChanged_;

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

		QModelIndex FindIndex (const QString& path) const;
		QModelIndex FindIndexByFileName (const QString& name) const;

		QList<QPair<MediaInfo, MediaInfo>> GetModified () const;
	private:
		QList<File>::iterator FindFile (const QString&);
		QList<File>::const_iterator FindFile (const QString&) const;
	};
}
}
}
