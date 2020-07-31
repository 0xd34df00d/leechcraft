/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "desaturateeffect.h"
#include <limits>
#include <cmath>
#include <QPainter>

namespace LC
{
namespace SB2
{
	DesaturateEffect::DesaturateEffect (QObject *parent)
	: QGraphicsEffect (parent)
	, Strength_ (0)
	{
	}

	qreal DesaturateEffect::GetStrength () const
	{
		return Strength_;
	}

	void DesaturateEffect::SetStrength (qreal strength)
	{
		if (std::fabs (static_cast<float> (strength) - Strength_) < std::numeric_limits<qreal>::epsilon ())
			return;

		Strength_ = static_cast<float> (strength);
		emit strengthChanged ();

		update ();
	}

	void DesaturateEffect::draw (QPainter *painter)
	{
		if (std::fabs (Strength_) < 0.001)
		{
			drawSource (painter);
			return;
		}

		QPoint offset;
		const auto& px = sourcePixmap (Qt::LogicalCoordinates, &offset);

		auto img = px.toImage ();
		switch (img.format ())
		{
		case QImage::Format_ARGB32:
		case QImage::Format_ARGB32_Premultiplied:
			break;
		default:
			img = img.convertToFormat (QImage::Format_ARGB32);
			break;
		}
		img.detach ();

		const auto height = img.height ();
		const auto width = img.width ();
		for (int y = 0; y < height; ++y)
		{
			const auto scanline = reinterpret_cast<QRgb*> (img.scanLine (y));
			for (int x = 0; x < width; ++x)
			{
				auto& color = scanline [x];
				const auto grayPart = qGray (color) * Strength_;
				const auto r = qRed (color) * (1 - Strength_) + grayPart;
				const auto g = qGreen (color) * (1 - Strength_) + grayPart;
				const auto b = qBlue (color) * (1 - Strength_) + grayPart;
				color = qRgba (r, g, b, qAlpha (color));
			}
		}

		painter->drawImage (offset, img);
	}
}
}
