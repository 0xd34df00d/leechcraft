/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

namespace LC::Azoth::Transfers
{
	enum class DeofferReason
	{
		Expired,
		Accepted,
		Declined,
	};

	struct JobContext
	{
		struct In { QString SavePath_; };
		struct Out {};
		std::variant<In, Out> Dir_;

		QString OrigFilename_;
		qint64 Size_;

		QString EntryName_;
		QString EntryId_;
	};
}
