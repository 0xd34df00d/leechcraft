/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "scriptobject.h"
#include <QVariant>
#include "effects.h"

namespace LC
{
namespace Poshuku
{
namespace DCAC
{
	ScriptObject::ScriptObject (QObject *parent)
	: QObject { parent }
	{
	}

	QVariant ScriptObject::invert (int threshold) const
	{
		return QVariant::fromValue<Effect_t> (InvertEffect { threshold });
	}

	QVariant ScriptObject::reduceLightness (double factor) const
	{
		return QVariant::fromValue<Effect_t> (LightnessEffect { factor });
	}

	QVariant ScriptObject::colorTemperature (int temperature) const
	{
		return QVariant::fromValue<Effect_t> (ColorTempEffect { temperature });
	}
}
}
}
