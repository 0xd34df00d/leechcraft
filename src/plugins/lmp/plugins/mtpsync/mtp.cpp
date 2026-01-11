/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "mtp.h"
#include <QBuffer>
#include <QFileInfo>
#include <QImage>
#include <libmtp.h>
#include <taglib/fileref.h>
#include <util/sll/qtutil.h>
#include <util/sys/paths.h>
#include <interfaces/lmp/itagresolver.h>
#include "mtphelpers.h"

namespace LC::LMP::MTPSync
{
	size_t qHash (const UsbDevice& device)
	{
		return ::qHash (std::pair { device.Bus_, device.Dev_ });
	}

	MtpRunner::MtpRunner ()
	{
		Thread_.setServiceLevel (QThread::QualityOfService::Eco);
		Thread_.start (QThread::LowPriority);
		Mtp_.moveToThread (&Thread_);

		connect (&Mtp_,
				&Mtp::mtpConnected,
				this,
				&MtpRunner::mtpConnected);
		connect (&Mtp_,
				&Mtp::mtpDisconnected,
				this,
				&MtpRunner::mtpDisconnected);
	}

	MtpRunner::~MtpRunner ()
	{
		QMetaObject::invokeMethod (&Mtp_,
				[this, thread = thread ()] { Mtp_.moveToThread (thread); },
				Qt::BlockingQueuedConnection);
		Thread_.quit ();
		Thread_.wait ();
	}

	Mtp::Mtp ()
	{
		connect (this,
				&Mtp::mtpDisconnected,
				this,
				[this] (const QList<MtpDeviceInfo>& devices)
				{
					for (const auto& device : devices)
						CacheSerial2MtpDev_.remove (device.Serial_);
				});
	}

	namespace
	{
		bool Check (const UsbDevice& device)
		{
			return LIBMTP_Check_Specific_Device (device.Bus_, device.Dev_);
		}

		QHash<UsbDevice, MtpDeviceInfo> GetDevices ()
		{
			QHash<UsbDevice, MtpDeviceInfo> infos;

			const auto [raws, count] = Helpers::GetRawDevices ();
			qDebug () << "detected" << count << "devices";
			for (int i = 0; i < count; ++i)
			{
				const auto device = Helpers::OpenRawDevice (raws [i]);
				if (!device)
					continue;

				const UsbDevice usbDevice
				{
					.Bus_ = static_cast<int> (raws [i].bus_location),
					.Dev_ = static_cast<int> (raws [i].devnum)
				};

				const auto& manufacturer = QString::fromUtf8 (LIBMTP_Get_Manufacturername (&*device));
				const auto& model = QString::fromUtf8 (LIBMTP_Get_Modelname (&*device));
				const auto& serial = QByteArray { LIBMTP_Get_Serialnumber (&*device) };
				const auto& devName = manufacturer + ' ' + model + ' ' + LIBMTP_Get_Friendlyname (&*device);
				const MtpDeviceInfo info
				{
					.UsbDevice_ = usbDevice,
					.DevName_ = devName.simplified ().trimmed (),
					.Serial_ = serial,
					.Formats_ = Helpers::GetSupportedFormats (*device),
					.Storages_ = Helpers::GetStorages (*device),
				};
				infos [usbDevice] = info;
			}

			return infos;
		}

		template<typename K, typename V>
		QList<V> Subkeys (const QSet<K>& keys, const QHash<K, V>& hash)
		{
			QList<V> result;
			for (const auto& key : keys)
				result << hash [key];
			return result;
		}
	}

	QList<MtpDeviceInfo> Mtp::GetCurrentDevices () const
	{
		return Devices_.values ();
	}

	void Mtp::Refresh ()
	{
		auto devices = GetDevices ();

		const QSet<UsbDevice> oldDevs { Devices_.keyBegin (), Devices_.keyEnd ()};
		const QSet<UsbDevice> newDevs { devices.keyBegin (), devices.keyEnd () };

		if (const auto added = newDevs - oldDevs;
			!added.isEmpty ())
			emit mtpConnected (Subkeys (added, devices));

		if (const auto removed = oldDevs - newDevs;
			!removed.isEmpty ())
			emit mtpDisconnected (Subkeys (removed, Devices_));

		Devices_ = std::move (devices);
	}

	void Mtp::HandleDevicesDisconnected (const QList<UsbDevice>& devices)
	{
		QList<MtpDeviceInfo> mtps;
		for (const auto& dev : devices)
			if (Devices_.contains (dev))
				mtps << Devices_.take (dev);

		if (!mtps.isEmpty ())
			emit mtpDisconnected (mtps);
	}

	void Mtp::HandleDevicesConnected (const QList<UsbDevice>& devices)
	{
		if (std::ranges::any_of (devices, Check))
			Refresh ();
	}

	namespace
	{
		std::shared_ptr<QFile> MakeTemporaryFile (const QString& origPath)
		{
			const auto& suffix = QFileInfo { origPath }.completeSuffix ();
			const auto& tmpPath = Util::GetTemporaryName ("lc_lmp_mtpsync.XXXXXX."_qs + suffix);
			if (!QFile::copy (origPath, tmpPath))
			{
				qWarning () << "unable to copy" << origPath << "to" << tmpPath;
				return {};
			}

			return
			{
				new QFile { tmpPath },
				[] (QFile *file)
				{
					file->remove ();
					delete file;
				}
			};
		}

		TagLib::ByteVector GetImageBytes (const QString& path)
		{
			const QImage image { path };
			if (image.isNull ())
			{
				qWarning () << "unable to load image from" << path;
				return {};
			}

			QBuffer buffer;
			buffer.open (QIODevice::WriteOnly);
			image.save (&buffer, "JPG", 90);
			const auto& data = buffer.data ();

			return { data.data (), static_cast<unsigned int> (data.size ()) };
		}

		bool DoEmbed (const QString& path, const QString& albumArtPath, ITagResolver& resolver)
		{
			const auto& albumArt = GetImageBytes (albumArtPath);
			if (albumArt.isEmpty ())
				return false;

			using namespace std::string_literals;
			TagLib::VariantMap pic
			{
				{ "data"s, albumArt },
				{ "mimeType"s, TagLib::String { "image/jpeg"s } },
				{ "pictureType"s, TagLib::String { "Front Cover"s } },
			};

			const QMutexLocker guard { &resolver.GetMutex () };
			TagLib::FileRef fileRef { path.toUtf8 ().constData (), true, TagLib::AudioProperties::Fast };
			if (fileRef.isNull ())
			{
				qWarning () << "unable to create TagLib ref for" << path;
				return false;
			}

			if (!fileRef.setComplexProperties ("PICTURE"s, { pic }))
			{
				qWarning () << "unable to set PICTURE for" << path;
				return false;
			}

			return fileRef.save ();
		}

		std::shared_ptr<QFile> EmbedAlbumArt (const QString& origPath, const QString& albumArtPath, ITagResolver& resolver)
		{
			if (albumArtPath.isEmpty () || !QFile::exists (albumArtPath))
				return {};

			const auto& tmpFile = MakeTemporaryFile (origPath);
			if (!tmpFile || !DoEmbed (tmpFile->fileName (), albumArtPath, resolver))
				return {};

			return tmpFile;
		}
	}

	ISyncPlugin::UploadResult Mtp::Upload (const UploadCtx& ctx)
	{
		const auto device = GetDevice (ctx.Serial_);
		if (!device)
			return { Util::AsLeft, { QFile::ResourceError, tr ("Unable to open device %1.").arg (ctx.Serial_) } };

		const auto track = Helpers::New (LIBMTP_new_track_t (), &LIBMTP_destroy_track_t);
		Helpers::FillTrack (*track, ctx.MediaInfo_);
		track->storage_id = ctx.StorageId_;

		const auto file = EmbedAlbumArt (ctx.LocalPath_, ctx.AlbumArtPath_, ctx.TagsResolver_);
		const auto filename = file ? file->fileName () : ctx.LocalPath_;

		using namespace std::chrono_literals;
		constexpr auto retries = 3;
		constexpr auto backoff = 500ms;
		bool isSuccessful = false;
		for (int i = 0; i < retries && !isSuccessful; ++i)
			if (const auto err = LIBMTP_Send_Track_From_File (&*device, filename.toUtf8 ().constData (), &*track, nullptr, nullptr))
			{
				qWarning () << "sending track failed:" << err << "; attempt" << i;
				LIBMTP_Dump_Errorstack (&*device);
				LIBMTP_Clear_Errorstack (&*device);
				QThread::sleep (backoff * (i + 1));
			}
			else
				isSuccessful = true;

		if (!isSuccessful)
			return { Util::AsLeft, { QFile::WriteError, tr ("Error sending track to the device.") } };

		return ISyncPlugin::UploadSuccess {};
	}

	LibMtpDevice_ptr Mtp::GetDevice (const QByteArray& serial)
	{
		auto& device = CacheSerial2MtpDev_ [serial];
		if (!device)
			device = Helpers::GetDeviceBySerial (serial);
		if (!device)
			qWarning () << "unable to open device for" << serial;
		return device;
	}
}
