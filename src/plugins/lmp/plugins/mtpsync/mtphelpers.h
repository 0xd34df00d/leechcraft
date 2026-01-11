/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <libmtp.h>
#include "types.h"

namespace LC::LMP
{
	struct MediaInfo;
}

namespace LC::LMP::MTPSync::Helpers
{
	template<typename T>
	using LibMtpHeapPtr = std::unique_ptr<T, decltype (&free)>;

	template<typename T>
	struct LibMtpArray
	{
		LibMtpHeapPtr<T []> Devices_;
		int Count_;
	};

	LibMtpArray<LIBMTP_raw_device_t> GetRawDevices ();

	LibMtpArray<uint16_t> GetSupportedFiletypes (LIBMTP_mtpdevice_t& device);
	LibMtpDevice_ptr OpenRawDevice (LIBMTP_raw_device_t& raw);
	LibMtpDevice_ptr GetDeviceBySerial (QByteArray serial);

	std::optional<int> GetBatteryPercentage (LIBMTP_mtpdevice_t& device);

	QList<DeviceStorage> GetStorages (LIBMTP_mtpdevice_t& device);
	LIBMTP_devicestorage_t* GetStorage (LIBMTP_mtpdevice_t& device, uint32_t id);

	QStringList GetSupportedFormats (LIBMTP_mtpdevice_t& device);
	LIBMTP_filetype_t GetFileType (const QString& format);

	void FillTrack (LIBMTP_track_t& track, const MediaInfo& info);

	using LibMtpAlbumPtr = std::unique_ptr<LIBMTP_album_t, decltype (&LIBMTP_destroy_album_t)>;
	LibMtpAlbumPtr FindOrCreateAlbum (LIBMTP_mtpdevice_t& device, const MediaInfo&);

	bool AppendTrack (LIBMTP_mtpdevice_t& device, const LIBMTP_track_t& track, LIBMTP_album_t& album);

	void SetAlbumArt (LIBMTP_mtpdevice_t& device, const LIBMTP_album_t& album, const QString& localPath);

	template<typename T, typename F>
	std::unique_ptr<T, F> New (T *t, F f)
	{
		return { t, f };
	}
}
