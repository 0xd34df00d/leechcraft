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

#include "mtpsync.h"
#include <map>
#include <QIcon>
#include <QTimer>

namespace LeechCraft
{
namespace LMP
{
namespace MTPSync
{
	void Plugin::Init (ICoreProxy_ptr)
	{
		LIBMTP_Init ();

		QTimer::singleShot (5000,
				this,
				SLOT (pollDevices ()));
	}

	void Plugin::SecondInit ()
	{
	}

	void Plugin::Release ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.LMP.MTPSync";
	}

	QString Plugin::GetName () const
	{
		return "LMP MTPSync";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Adds support for synchronization with MTP-enabled portable media players.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.LMP.CollectionSync";
		return result;
	}

	void Plugin::SetLMPProxy (ILMPProxy_ptr proxy)
	{
		LMPProxy_ = proxy;
	}

	QString Plugin::GetSyncSystemName () const
	{
		return "MTP";
	}

	QObject* Plugin::GetObject ()
	{
		return this;
	}

	UnmountableDevInfos_t Plugin::AvailableDevices () const
	{
		return Infos_;
	}

	void Plugin::SetFileInfo (const QString& origLocalPath, const UnmountableFileInfo& info)
	{
		OrigInfos_ [origLocalPath] = info;
	}

	void Plugin::Upload (const QString& localPath, const QString&, const QByteArray& devId, const QByteArray& storageId)
	{
		bool found = false;

		LIBMTP_raw_device_t *rawDevices;
		int numRawDevices = 0;
		LIBMTP_Detect_Raw_Devices (&rawDevices, &numRawDevices);
		for (int i = 0; i < numRawDevices; ++i)
		{
			std::shared_ptr<LIBMTP_mtpdevice_t> device (LIBMTP_Open_Raw_Device (&rawDevices [i]), LIBMTP_Release_Device);
			if (!device)
				continue;

			const auto& serial = LIBMTP_Get_Serialnumber (device.get ());
			if (serial == devId)
			{
				UploadTo (device.get (), storageId, localPath);
				found = true;
				break;
			}
		}
		free (rawDevices);

		if (!found)
			emit uploadFinished (localPath,
					QFile::ResourceError,
					tr ("Unable to find the requested device."));
	}

	void Plugin::HandleTransfer (const QString& path, quint64 sent, quint64 total)
	{
		if (sent == total)
			emit uploadFinished (path, QFile::NoError, QString ());
	}

	namespace
	{
		struct CallbackData
		{
			Plugin *Plugin_;
			QString LocalPath_;
		};

		int TransferCallback (uint64_t sent, uint64_t total, const void *rawData)
		{
			qDebug () << sent << total;
			auto data = static_cast<const CallbackData*> (rawData);

			data->Plugin_->HandleTransfer (data->LocalPath_, sent, total);

			if (sent == total)
				delete data;

			return 0;
		}
	}

	void Plugin::UploadTo (LIBMTP_mtpdevice_t *device, const QByteArray& storageId, const QString& localPath)
	{
		LIBMTP_Get_Storage (device, 0);

		auto storage = device->storage;

		while (storage)
		{
			if (QByteArray::number (storage->id) == storageId)
				break;
			storage = storage->next;
		}

		if (!storage)
		{
			emit uploadFinished (localPath,
					QFile::ResourceError,
					tr ("Unable to find the requested storage."));
			return;
		}

		qDebug () << "uploading" << localPath << "to" << storage->id;

		const auto id = storage->id;
		auto album = GetAlbum (device, localPath, id);

		auto track = LIBMTP_new_track_t ();
		track->parent_id = album ? album->album_id : 0;
		track->storage_id = id;

		qDebug () << LIBMTP_Send_Track_From_File (device, localPath.toUtf8 ().constData (), track, TransferCallback, new CallbackData ({ this, localPath }));
		LIBMTP_destroy_track_t (track);
	}

	namespace
	{
		bool IsRightAlbum (const UnmountableFileInfo& info, const LIBMTP_album_t *album)
		{
			return info.Artist_ == album->artist &&
					info.Album_ == album->name &&
					info.Genres_.join ("; ") == album->genre;
		}
	}

	LIBMTP_album_t* Plugin::GetAlbum (LIBMTP_mtpdevice_t *device, const QString& localPath, uint32_t storageId)
	{
		if (!OrigInfos_.contains (localPath))
			return 0;

		const auto& info = OrigInfos_.take (localPath);
		const auto pos = std::find_if (ExistingAlbums_.begin (), ExistingAlbums_.end (),
				[&info] (const LIBMTP_album_t *album) { return IsRightAlbum (info, album); });
		if (pos != ExistingAlbums_.end ())
			return *pos;

		auto album = LIBMTP_new_album_t ();

		album->artist = strdup (info.Artist_.toUtf8 ().constData ());
		album->name = strdup (info.Album_.toUtf8 ().constData ());
		album->genre = strdup (info.Genres_.join ("; ").toUtf8 ().constData ());

		album->album_id = 0;
		album->next = 0;
		album->no_tracks = 0;
		album->parent_id = 0;
		album->storage_id = storageId;
		album->tracks = 0;

		if (!LIBMTP_Create_New_Album (device, album))
		{
			ExistingAlbums_ << album;
			return album;
		}

		qWarning () << Q_FUNC_INFO << "unable to create album";

		LIBMTP_destroy_album_t (album);
		return 0;
	}

	namespace
	{
		QStringList GetSupportedFormats (LIBMTP_mtpdevice_t *device)
		{
			static const std::map<uint16_t, QString> formatID2Format =
			{
				{ LIBMTP_FILETYPE_MP3, "mp3" },
				{ LIBMTP_FILETYPE_MP4, "mp4" },
				{ LIBMTP_FILETYPE_OGG, "ogg" },
				{ LIBMTP_FILETYPE_ASF, "asf" },
				{ LIBMTP_FILETYPE_AAC, "aac" },
				{ LIBMTP_FILETYPE_FLAC, "flac" }
			};

			QStringList result;

			uint16_t *formats = 0;
			uint16_t formatsLength = 0;
			LIBMTP_Get_Supported_Filetypes (device, &formats, &formatsLength);
			for (uint16_t i = 0; i < formatsLength; ++i)
			{
				auto format = formats [i];
				const auto pos = formatID2Format.find (format);
				if (pos == formatID2Format.end ())
				{
					qWarning () << Q_FUNC_INFO
							<< "unknown format"
							<< format;
					continue;
				}
				result << pos->second;
			}
			free (formats);

			return result;
		}

		QList<UnmountablePartition> GetPartitions (LIBMTP_mtpdevice_t *device)
		{
			LIBMTP_Get_Storage (device, LIBMTP_STORAGE_SORTBY_MAXSPACE);
			auto storage = device->storage;

			QList<UnmountablePartition> result;
			while (storage)
			{
				const UnmountablePartition part
				{
					QByteArray::number (storage->id),
					QString::fromUtf8 (storage->StorageDescription),
					storage->FreeSpaceInBytes,
					storage->MaxCapacity
				};
				result << part;
				storage = storage->next;
			}

			return result;
		}
	}

	void Plugin::pollDevices ()
	{
		UnmountableDevInfos_t infos;

		LIBMTP_raw_device_t *rawDevices;
		int numRawDevices = 0;
		LIBMTP_Detect_Raw_Devices (&rawDevices, &numRawDevices);
		for (int i = 0; i < numRawDevices; ++i)
		{
			auto device = LIBMTP_Open_Raw_Device (&rawDevices [i]);
			if (!device)
				continue;

			const auto& devName = QString::fromUtf8 (rawDevices [i].device_entry.vendor) + " " +
					QString::fromUtf8 (rawDevices [i].device_entry.product) + " " +
					LIBMTP_Get_Friendlyname (device);
			const UnmountableDevInfo info
			{
				LIBMTP_Get_Serialnumber (device),
				LIBMTP_Get_Manufacturername (device),
				devName.simplified ().trimmed (),
				GetPartitions (device),
				GetSupportedFormats (device)
			};
			infos << info;

			LIBMTP_Release_Device (device);
		}
		free (rawDevices);

		QTimer::singleShot (30000,
				this,
				SLOT (pollDevices ()));

		if (infos == Infos_)
			return;

		Infos_ = infos;
		emit availableDevicesChanged ();
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_lmp_mtpsync, LeechCraft::LMP::MTPSync::Plugin);
