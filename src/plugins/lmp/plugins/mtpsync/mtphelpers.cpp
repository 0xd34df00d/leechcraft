/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "mtphelpers.h"
#include <unordered_map>
#include <QBuffer>
#include <QFileInfo>
#include <QImage>
#include <QtDebug>
#include <util/sll/qtutil.h>
#include <util/lmp/mediainfo.h>

namespace LC::LMP::MTPSync::Helpers
{
	LibMtpArray<LIBMTP_raw_device_t> GetRawDevices ()
	{
		LIBMTP_raw_device_t *rawDevices = nullptr;
		int numRawDevices = 0;
		LIBMTP_Detect_Raw_Devices (&rawDevices, &numRawDevices);
		return { { rawDevices, &free }, numRawDevices };
	}

	LibMtpArray<uint16_t> GetSupportedFiletypes (LIBMTP_mtpdevice_t& device)
	{
		uint16_t *formats = nullptr;
		uint16_t numFormats = 0;
		LIBMTP_Get_Supported_Filetypes (&device, &formats, &numFormats);
		return { { formats, &free }, numFormats };
	}

	LibMtpDevice_ptr OpenRawDevice (LIBMTP_raw_device_t& raw)
	{
		return { LIBMTP_Open_Raw_Device (&raw), &LIBMTP_Release_Device };
	}

	LibMtpDevice_ptr GetDeviceBySerial (QByteArray serial)
	{
		return { LIBMTP_Get_Device_By_SerialNumber (serial.data ()), &LIBMTP_Release_Device };
	}

	std::optional<int> GetBatteryPercentage (LIBMTP_mtpdevice_t& device)
	{
		uint8_t maxBattLevel = 0, curBattLevel = 0;
		if (!LIBMTP_Get_Batterylevel (&device, &maxBattLevel, &curBattLevel) && curBattLevel)
			return 100 * curBattLevel / maxBattLevel;

		return {};
	}

	QList<DeviceStorage> GetStorages (LIBMTP_mtpdevice_t& device)
	{
		LIBMTP_Get_Storage (&device, LIBMTP_STORAGE_SORTBY_MAXSPACE);
		auto storage = device.storage;

		qDebug () << "initial storage:" << storage;
		QList<DeviceStorage> result;
		while (storage)
		{
			qDebug () << "storage" << storage << storage->id << "type" << storage->StorageType << storage->StorageDescription;
			const DeviceStorage part
			{
				storage->id,
				QString::fromUtf8 (storage->StorageDescription),
				storage->MaxCapacity,
			};
			result << part;
			storage = storage->next;
		}

		return result;
	}

	LIBMTP_devicestorage_t* GetStorage (LIBMTP_mtpdevice_t& device, uint32_t id)
	{
		if (!device.storage)
			if (LIBMTP_Get_Storage (&device, 0) == -1)
			{
				qWarning () << "failed to update storages for device" << LIBMTP_Get_Serialnumber (&device);
				LIBMTP_Dump_Errorstack (&device);
				LIBMTP_Clear_Errorstack (&device);
				return nullptr;
			}

		auto storage = device.storage;
		while (storage)
		{
			if (id == storage->id)
				return storage;
			storage = storage->next;
		}
		return nullptr;
	}

	QStringList GetSupportedFormats (LIBMTP_mtpdevice_t& device)
	{
		static const std::unordered_map<uint16_t, QString> formatID2Format =
		{
			{ LIBMTP_FILETYPE_MP3, "mp3"_qs },
			{ LIBMTP_FILETYPE_MP4, "mp4"_qs },
			{ LIBMTP_FILETYPE_OGG, "ogg"_qs },
			{ LIBMTP_FILETYPE_ASF, "asf"_qs },
			{ LIBMTP_FILETYPE_AAC, "aac"_qs },
			{ LIBMTP_FILETYPE_FLAC, "flac"_qs },
			{ LIBMTP_FILETYPE_WMA, "wma"_qs },

			// uninteresting formats go here
			{ LIBMTP_FILETYPE_FOLDER, {} },
			{ LIBMTP_FILETYPE_WMV, {} },
			{ LIBMTP_FILETYPE_AVI, {} },
			{ LIBMTP_FILETYPE_MPEG, {} },
			{ LIBMTP_FILETYPE_JPEG, {} }
		};

		const auto [formats, count] = GetSupportedFiletypes (device);

		QStringList result;
		for (uint16_t i = 0; i < count; ++i)
		{
			auto format = formats [i];
			const auto pos = formatID2Format.find (format);
			if (pos == formatID2Format.end ())
			{
				qWarning () << "unknown format" << format;
				continue;
			}
			if (!pos->second.isEmpty ())
				result << pos->second;
		}
		return result;
	}

	LIBMTP_filetype_t GetFileType (const QString& format)
	{
		static const QMap<QString, LIBMTP_filetype_t> map
		{
			{ "mp3", LIBMTP_FILETYPE_MP3 },
			{ "ogg", LIBMTP_FILETYPE_OGG },
			{ "aac", LIBMTP_FILETYPE_AAC },
			{ "flac", LIBMTP_FILETYPE_FLAC },
			{ "wma", LIBMTP_FILETYPE_WMA },
		};
		return map.value (format, LIBMTP_FILETYPE_UNDEF_AUDIO);
	}

	namespace
	{
		auto GetStr (const QString& str)
		{
			return strdup (str.toUtf8 ().constData ());
		}
	}

	void FillTrack (LIBMTP_track_t& track, const MediaInfo& info)
	{
		const QFileInfo fi { info.LocalPath_ };
		track.filename = GetStr (fi.fileName ());
		track.album = GetStr (info.Album_);
		track.title = GetStr (info.Title_);
		track.genre = GetStr (info.Genres_.join ("; "_qs));
		track.artist = GetStr (info.Artist_);
		track.tracknumber = info.TrackNumber_;
		track.filetype = GetFileType (fi.suffix ());
		track.filesize = fi.size ();
		track.date = GetStr (QString::number (info.Year_) + "0101T0000.0"_qs);
	}
}
