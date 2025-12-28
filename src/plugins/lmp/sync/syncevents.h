/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <variant>
#include <QFile>
#include <QString>

namespace LC::LMP::SyncEvents
{
	struct SyncData
	{
		QString Orig_;
	};

	struct TranscodingData : SyncData
	{
		QString Target_;
	};

	struct XcodingStarted : TranscodingData {};
	struct XcodingFinished : TranscodingData {};
	struct XcodingFailed : TranscodingData
	{
		QString Message_;
	};

	struct CopyData : SyncData
	{
		QString CopySource_;
	};

	struct CopyStarted : CopyData {};
	struct CopyFinished : CopyData {};
	struct CopyFailed : CopyData
	{
		QFile::FileError Reason_;
		QString Message_;
	};

	using Event = std::variant<
			XcodingStarted, XcodingFinished, XcodingFailed,
			CopyStarted, CopyFinished, CopyFailed
		>;
}
