/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QStringList>
#include <QSet>

inline uint qHash (const QStringList& list)
{
	return qHash (list.join ("<|>"));
}

namespace LC
{
namespace Snails
{
	enum class TaskPriority
	{
		High,
		Low
	};

	enum class MailListMode
	{
		Normal,
		MultiSelect
	};

	namespace Mimes
	{
		static const QString FolderPath { "x-leechcraft/snails-folder-path" };
		static const QString MessageIdList { "x-leechcraft/snails-message-id-list" };
	}
}
}
