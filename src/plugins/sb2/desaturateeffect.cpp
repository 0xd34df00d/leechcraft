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

#include "desaturateeffect.h"
#include <limits>
#include <QPainter>

namespace LeechCraft
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
		if (std::fabs (strength - Strength_) < std::numeric_limits<qreal>::epsilon ())
			return;

		Strength_ = strength;
		emit strengthChanged ();

		update ();
	}

	void DesaturateEffect::draw (QPainter *painter)
	{
		QPoint offset;
		auto px = sourcePixmap (Qt::DeviceCoordinates, &offset);

		const auto restoreTransform = painter->worldTransform ();
		painter->setWorldTransform (QTransform ());

		if (std::fabs (Strength_) >= std::numeric_limits<qreal>::epsilon ())
		{
			auto img = px.toImage ();
			for (int y = 0; y < img.height (); ++y)
				for (int x = 0; x < img.width (); ++x)
				{
					const auto color = img.pixel (x, y);
					const auto grayPart = qGray (color) * Strength_;
					const auto r = qRed (color) * (1 - Strength_) + grayPart;
					const auto g = qGreen (color) * (1 - Strength_) + grayPart;
					const auto b = qBlue (color) * (1 - Strength_) + grayPart;
					img.setPixel (x, y, qRgba (r, g, b, qAlpha (color)));
				}

			painter->drawImage (offset, img);
		}
		else
			painter->drawPixmap (offset, px);

		painter->setWorldTransform (restoreTransform);
	}
}
}
