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

#include "filesmodel.h"
#include <QtDebug>

namespace LeechCraft
{
namespace LMP
{
namespace Graffiti
{
	FilesModel::File::File (const QFileInfo& fi)
	: Path_ (fi.absoluteFilePath ())
	, Name_ (fi.fileName ())
	{
	}

	FilesModel::FilesModel (QObject *parent)
	: QAbstractItemModel (parent)
	, Headers_ ({ tr ("Track"), tr ("Album"), tr ("Artist"), tr ("File name") })
	{
	}

	QModelIndex FilesModel::index (int row, int column, const QModelIndex& parent) const
	{
		return parent.isValid () ? QModelIndex () : createIndex (row, column);
	}

	QModelIndex FilesModel::parent (const QModelIndex&) const
	{
		return QModelIndex ();
	}

	int FilesModel::rowCount (const QModelIndex& parent) const
	{
		return parent.isValid () ? 0 : Files_.size ();
	}

	int FilesModel::columnCount (const QModelIndex&) const
	{
		return Headers_.size ();
	}

	QVariant FilesModel::headerData (int section, Qt::Orientation orientation, int role) const
	{
		if (orientation != Qt::Horizontal)
			return QVariant ();

		if (role != Qt::DisplayRole)
			return QVariant ();

		return Headers_.at (section);
	}

	QVariant FilesModel::data (const QModelIndex& index, int role) const
	{
		if (role != Qt::DisplayRole)
			return QVariant ();

		const auto& file = Files_.at (index.row ());
		switch (index.column ())
		{
		case Columns::Filename:
			return file.Name_;
		case Columns::Artist:
			return file.Artist_;
		case Columns::Album:
			return file.Album_;
		case Columns::Title:
			return file.Title_;
		}

		qWarning () << Q_FUNC_INFO
				<< "unknown column"
				<< index.column ();
		return QVariant ();
	}

	void FilesModel::AddFiles (const QList<QFileInfo>& files)
	{
		if (files.isEmpty ())
			return;

		beginInsertRows (QModelIndex (), Files_.size (), files.size () + Files_.size ());
		std::copy (files.begin (), files.end (), std::back_inserter (Files_));
		endInsertRows ();
	}

	void FilesModel::SetInfos (const QList<MediaInfo>& infos)
	{
		for (const auto& info : infos)
		{
			const auto pos = std::find_if (Files_.begin (), Files_.end (),
					[&info] (const File& file) { return file.Path_ == info.LocalPath_; });
			if (pos == Files_.end ())
				continue;

			pos->Title_ = info.Title_;
			pos->Album_ = info.Album_;
			pos->Artist_ = info.Artist_;

			const auto row = std::distance (Files_.begin (), pos);
			emit dataChanged (index (row, 0), index (row, Columns::MaxColumn - 1));
		}
	}

	void FilesModel::Clear ()
	{
		if (Files_.isEmpty ())
			return;

		beginRemoveRows (QModelIndex (), 0, Files_.size ());
		Files_.clear ();
		endRemoveRows ();
	}
}
}
}
