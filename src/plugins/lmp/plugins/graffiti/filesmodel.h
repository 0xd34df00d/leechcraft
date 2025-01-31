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
#include <util/lmp/mediainfo.h>

namespace LC::LMP::Graffiti
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
			bool IsChanged_ = false;

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

		explicit FilesModel (QObject*);

		QModelIndex index (int, int, const QModelIndex& = QModelIndex ()) const override;
		QModelIndex parent (const QModelIndex&) const override;
		int rowCount (const QModelIndex&) const override;
		int columnCount (const QModelIndex&) const override;
		QVariant headerData (int, Qt::Orientation, int) const override;
		QVariant data (const QModelIndex&, int) const override;

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
