/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QString>

using LIBMTP_album_t = struct LIBMTP_album_struct;
using LIBMTP_mtpdevice_t = struct LIBMTP_mtpdevice_struct;

namespace LC::LMP::MTPSync
{
	struct DeviceStorage
	{
		uint32_t ID_;
		QString Name_;
		quint64 TotalSize_;
	};

	using LibMtpDevice_ptr = std::shared_ptr<LIBMTP_mtpdevice_t>;
}
