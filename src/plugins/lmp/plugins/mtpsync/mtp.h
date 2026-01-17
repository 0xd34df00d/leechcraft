/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <util/threads/coro.h>
#include <util/threads/coro/metamethod.h>
#include <util/threads/coro/workerthread.h>
#include <interfaces/lmp/isyncplugin.h>
#include "types.h"

namespace LC::LMP
{
	class ITagResolver;
}

namespace LC::LMP::MTPSync
{
	struct UsbDevice
	{
		int Bus_;
		int Dev_;

		constexpr auto operator<=> (const UsbDevice&) const = default;
	};

	size_t qHash (const UsbDevice&);

	struct MtpDeviceInfo
	{
		UsbDevice UsbDevice_;

		QString DevName_;
		QByteArray Serial_;

		QStringList Formats_;
		QList<DeviceStorage> Storages_;
	};

	class Mtp : public QObject
	{
		Q_OBJECT

		QHash<UsbDevice, MtpDeviceInfo> Devices_;
		QHash<QByteArray, LibMtpDevice_ptr> CacheSerial2MtpDev_;
	public:
		explicit Mtp ();

		QList<MtpDeviceInfo> GetCurrentDevices () const;

		void Refresh ();

		void HandleDevicesConnected (const QList<UsbDevice>&);
		void HandleDevicesDisconnected (const QList<UsbDevice>&);

		struct UploadCtx
		{
			QByteArray Serial_;
			uint32_t StorageId_;

			QString LocalPath_;

			MediaInfo MediaInfo_;
			QString AlbumArtPath_;

			ITagResolver& TagsResolver_;
		};

		ISyncPlugin::UploadResult Upload (const UploadCtx&);
	private:
		LibMtpDevice_ptr GetDevice (const QByteArray& serial);
	signals:
		void mtpConnected (const QList<MtpDeviceInfo>&);
		void mtpDisconnected (const QList<MtpDeviceInfo>&);
	};

	class MtpRunner final : public Util::Coro::WorkerThread<Mtp, MtpRunner>
	{
		Q_OBJECT
	public:
		MtpRunner ();
	signals:
		void mtpConnected (const QList<MtpDeviceInfo>&);
		void mtpDisconnected (const QList<MtpDeviceInfo>&);
	};
}
