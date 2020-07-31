/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QStringList>
#include <QMetaType>
#include <QFile>
#include <QtPlugin>

namespace LC
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

		int BatteryCharge_;

		inline bool operator== (const UnmountableDevInfo& other) const
		{
			return ID_ == other.ID_;
		}

		inline bool operator< (const UnmountableDevInfo& other) const
		{
			return ID_ < other.ID_;
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

		virtual QObject* GetQObject () = 0;

		virtual void SetFileInfo (const QString& origLocalPath, const UnmountableFileInfo& info) = 0;

		virtual void Upload (const QString& localPath, const QString& origLocalPath,
				const QByteArray& devId, const QByteArray& storageId) = 0;

		virtual void Refresh () = 0;
	protected:
		virtual void availableDevicesChanged () = 0;

		virtual void uploadProgress (qint64, qint64) = 0;

		virtual void uploadFinished (const QString&, QFile::FileError, const QString&) = 0;
	};
}
}

Q_DECLARE_METATYPE (LC::LMP::UnmountableDevInfo)
Q_DECLARE_INTERFACE (LC::LMP::IUnmountableSync, "org.LeechCraft.LMP.IUnmountableSync/1.0")
