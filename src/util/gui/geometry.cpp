/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
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
