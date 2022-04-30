/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QPair>

namespace LC::Lemon
{
	class PlatformBackend : public QObject
	{
	public:
		using QObject::QObject;

		struct CurrentTrafficState
		{
			qint64 Down_;
			qint64 Up_;
		};

		virtual CurrentTrafficState GetCurrentNumBytes (const QString&) const = 0;
		virtual void Update (const QStringList&) = 0;
	};
}
