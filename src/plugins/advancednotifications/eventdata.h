/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QStringList>
#include <QPixmap>
#include <interfaces/structures.h>

namespace LC::AdvancedNotifications
{
	struct EventData
	{
		QByteArray SenderId_;
		QString EventId_;
		int Count_;
		QString Category_;
		QStringList VisualPath_;
		QString ExtendedText_;
		QString FullText_;
		QPixmap Pixmap_;

		QObject_ptr HandlingObject_;
		QStringList Actions_;

		Entity Canceller_;
	};

	struct EventKey
	{
		QByteArray SenderId_;
		QString EventId_;

		auto operator<=> (const EventKey&) const = default;
	};
}
