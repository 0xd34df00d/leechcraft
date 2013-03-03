/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#pragma once

#include <QPoint>
#include <util/utilconfig.h>

class QSize;
class QRect;

namespace LeechCraft
{
namespace Util
{
	enum FitFlag
	{
		NoFlags,
		NoOverlap = 0x01
	};

	Q_DECLARE_FLAGS (FitFlags, FitFlag);

	UTIL_API QPoint FitRectScreen (QPoint pos, const QSize& size,
			FitFlags flags = NoFlags, const QPoint& shiftAdd = QPoint (0, 0));
	UTIL_API QPoint FitRect (QPoint pos, const QSize& size, const QRect& geometry,
			FitFlags flags = NoFlags, const QPoint& shiftAdd = QPoint (0, 0));
}
}

Q_DECLARE_OPERATORS_FOR_FLAGS (LeechCraft::Util::FitFlags);
