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

	UnmountableDevInfos_t Plugin::AvailableDevices () const
	{
		return Infos_;
	}

	void Plugin::Upload (const QString& localPath, const QString& origLocalPath, const QByteArray& devId)
	{
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
