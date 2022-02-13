/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "colorthemeproxy.h"
#include <QtDebug>
#include <interfaces/core/icolorthememanager.h>

namespace LC::Util
{
	ColorThemeProxy::ColorThemeProxy (IColorThemeManager *manager, QObject *parent)
	: QObject (parent)
	, Manager_ (manager)
	{
		connect (manager->GetQObject (),
				SIGNAL (themeChanged ()),
				this,
				SIGNAL (colorsChanged ()));
	}

	QColor ColorThemeProxy::setAlpha (QColor color, qreal alpha)
	{
		color.setAlphaF (alpha);
		return color;
	}

	QColor ColorThemeProxy::GetColor (const QByteArray& group, const QByteArray& color) const
	{
		return Manager_->GetQMLColor (group, color);
	}
}
