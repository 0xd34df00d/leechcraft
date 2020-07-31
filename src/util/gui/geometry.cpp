/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "geometry.h"
#include <QGuiApplication>
#include <QRect>
#include <QScreen>
#include <QSize>
#include <QtDebug>

namespace LC::Util
{
	QPoint FitRectScreen (QPoint pos, const QSize& size, FitFlags flags, const QPoint& shiftAdd)
	{
		return FitRect (pos, size, ScreenGeometry (pos), flags, shiftAdd);
	}

	QPoint FitRect (QPoint pos, const QSize& size, const QRect& geometry,
			FitFlags flags, const QPoint& shiftAdd)
	{
		int xDiff = std::max (0, pos.x () + size.width () - (geometry.width () + geometry.x ()));
		if (!xDiff)
			xDiff = std::min (0, pos.x () - geometry.x ());
		int yDiff = std::max (0, pos.y () + size.height () - (geometry.height () + geometry.y ()));
		if (!yDiff)
			yDiff = std::min (0, pos.y () - geometry.y ());

		if (flags & FitFlag::NoOverlap)
		{
			auto overlapFixer = [] (int& diff, int dim)
			{
				if (diff > 0)
					diff = dim > diff ? dim : diff;
			};

			if (QRect (pos - QPoint (xDiff, yDiff), size).contains (pos) && yDiff < size.height ())
				overlapFixer (yDiff, size.height ());
			if (QRect (pos - QPoint (xDiff, yDiff), size).contains (pos) && xDiff < size.width ())
				overlapFixer (xDiff, size.width ());
		}

		if (xDiff)
			pos.rx () -= xDiff + shiftAdd.x ();
		if (yDiff)
			pos.ry () -= yDiff + shiftAdd.y ();

		return pos;
	}

	QScreen* GetScreenWithFallback (const QPoint& p)
	{
		if (auto screen = QGuiApplication::screenAt (p))
			return screen;

		qWarning () << Q_FUNC_INFO
				<< "unknown screen for point"
				<< p;
		return QGuiApplication::primaryScreen ();
	}

	QRect AvailableGeometry (const QPoint& p)
	{
		return GetScreenWithFallback (p)->availableGeometry ();
	}

	QRect ScreenGeometry (const QPoint& p)
	{
		return GetScreenWithFallback (p)->geometry ();
	}
}
