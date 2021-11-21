/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <optional>
#include <variant>
#include "localtypes.h"

class QUrl;

namespace LC::Azoth::Acetamide
{
	struct NickOnly
	{
		QString Nick_;

		bool operator== (const NickOnly&) const = default;
	};

	struct NickInfo
	{
		QString Nick_;
		QString User_;
		QString HostMask_;

		bool operator== (const NickInfo&) const = default;
	};

	struct UserInfo
	{
		QString User_;
		QString ServerName_;

		bool operator== (const UserInfo&) const = default;
	};

	struct ChannelTarget
	{
		ChannelOptions Opts_;
		bool HasPassword_ = false;

		bool operator== (const ChannelTarget&) const = default;
	};

	struct NoTarget
	{
		bool operator== (const NoTarget&) const = default;
	};

	using Target = std::variant<NoTarget, ChannelTarget, NickOnly, NickInfo, UserInfo>;

	struct DecodedUrl
	{
		ServerOptions Server_;
		bool HasServerPassword_ = false;

		Target Target_;
	};

	std::optional<DecodedUrl> DecodeUrl (const QUrl&);
}
