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
#include <interfaces/lmp/isyncplugin.h>
#include "types.h"

using LIBMTP_album_t = struct LIBMTP_album_struct;
using LIBMTP_mtpdevice_t = struct LIBMTP_mtpdevice_struct;

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
	public:
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
		};

		ISyncPlugin::UploadResult Upload (const UploadCtx&);
	signals:
		void mtpConnected (const QList<MtpDeviceInfo>&);
		void mtpDisconnected (const QList<MtpDeviceInfo>&);
	};

	class MtpRunner final : public QObject
	{
		Q_OBJECT

		QThread Thread_;
		Mtp Mtp_;
	public:
		MtpRunner ();
		~MtpRunner () override;

		MtpRunner (const MtpRunner&) = delete;
		MtpRunner (MtpRunner&&) = delete;
		MtpRunner& operator= (const MtpRunner&) = delete;
		MtpRunner& operator= (MtpRunner&&) = delete;

		template<typename F, typename... Args, typename R = std::invoke_result_t<F, Mtp*, Args...>>
		Util::ContextTask<R> Run (F&& f, Args&&... args)
		{
			co_return co_await Util::MetaMethod (Mtp_, std::forward<F> (f), std::forward<Args> (args)...);
		}
	signals:
		void mtpConnected (const QList<MtpDeviceInfo>&);
		void mtpDisconnected (const QList<MtpDeviceInfo>&);
	};
}
