/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QtGlobal>

namespace DebugHandler
{
	enum DebugWriteFlag
	{
		DWFNone = 0x00,
		DWFBacktrace = 0x01,
		DWFNoFileLog = 0x02
	};

	Q_DECLARE_FLAGS (DebugWriteFlags, DebugWriteFlag)

	void Write (QtMsgType type, const QMessageLogContext&, const char *message, DebugWriteFlags flags);
}

Q_DECLARE_OPERATORS_FOR_FLAGS (DebugHandler::DebugWriteFlags)
