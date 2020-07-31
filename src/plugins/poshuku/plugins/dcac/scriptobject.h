/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>

namespace LC
{
namespace Poshuku
{
namespace DCAC
{
	class ScriptObject : public QObject
	{
		Q_OBJECT
	public:
		ScriptObject (QObject* = nullptr);
	public slots:
		QVariant invert (int threshold = 0) const;
		QVariant reduceLightness (double factor) const;
		QVariant colorTemperature (int temperature) const;
	};
}
}
}
