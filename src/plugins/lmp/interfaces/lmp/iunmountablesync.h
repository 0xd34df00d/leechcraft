/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#include <QStringList>
#include <QMetaType>
#include <QFile>
#include <QtPlugin>

namespace LeechCraft
{
namespace LMP
{
	struct UnmountablePartition
	{
		QByteArray ID_;
		QString Name_;
		quint64 AvailableSize_;
		quint64 TotalSize_;
	};

	struct UnmountableDevInfo
	{
		QByteArray ID_;

		QString Manufacturer_;
		QString Name_;

		QList<UnmountablePartition> Partitions_;
		QStringList SupportedFormats_;

		inline bool operator== (const UnmountableDevInfo& other) const
		{
			return ID_ == other.ID_;
		}
	};

	typedef QList<UnmountableDevInfo> UnmountableDevInfos_t;

	struct UnmountableFileInfo
	{
		QString FileFormat_;

		int TrackNumber_;
		QString TrackTitle_;

		QString Artist_;
		QString Album_;
		int AlbumYear_;

		QString AlbumArtPath_;

		QStringList Genres_;
	};

	class IUnmountableSync
	{
	public:
		virtual ~IUnmountableSync () {}

		virtual UnmountableDevInfos_t AvailableDevices () const = 0;

		virtual QString GetSyncSystemName () const = 0;

		virtual QObject* GetObject () = 0;

		virtual void SetFileInfo (const QString& origLocalPath, const UnmountableFileInfo& info) = 0;

		virtual void Upload (const QString& localPath, const QString& origLocalPath,
				const QByteArray& devId, const QByteArray& storageId) = 0;
	protected:
		virtual void availableDevicesChanged () = 0;

		virtual void uploadFinished (const QString&, QFile::FileError, const QString&) = 0;
	};
}
}

Q_DECLARE_METATYPE (LeechCraft::LMP::UnmountableDevInfo)
Q_DECLARE_INTERFACE (LeechCraft::LMP::IUnmountableSync, "org.LeechCraft.LMP.IUnmountableSync/1.0");
