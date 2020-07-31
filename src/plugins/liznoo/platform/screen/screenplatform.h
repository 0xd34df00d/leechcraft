/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QSet>

namespace LC
{
namespace Liznoo
{
namespace Screen
{
	class ScreenPlatform : public QObject
	{
		Q_OBJECT
	protected:
		QSet<QString> ActiveProhibitions_;
	public:
		using QObject::QObject;

		virtual void ProhibitScreensaver (bool prohibit, const QString& id) = 0;
	};
}
}
}
