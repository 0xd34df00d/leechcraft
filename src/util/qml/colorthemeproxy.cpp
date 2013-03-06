/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "colorthemeproxy.h"
#include <QtDebug>
#include <interfaces/core/icolorthememanager.h>

namespace LeechCraft
{
namespace Util
{
	ColorThemeProxy::ColorThemeProxy (IColorThemeManager *manager, QObject *parent)
	: QObject (parent)
	, Manager_ (manager)
	{
		connect (manager->GetObject (),
				SIGNAL (themeChanged ()),
				this,
				SIGNAL (colorsChanged ()));
	}

	QColor ColorThemeProxy::setAlpha (QColor color, qreal alpha)
	{
		color.setAlphaF (alpha);
		return color;
	}

	QColor ColorThemeProxy::GetColor (const QString& group, const QString& color) const
	{
		return Manager_->GetQMLColor (group, color);
	}
}
}
