/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "effectprocessor.h"
#include <QPainter>
#include <QWidget>
#include <QtDebug>
#include <util/sll/visitor.h>
#include "invertcolors.h"
#include "reducelightness.h"
#include "colortemp.h"

namespace LC
{
namespace Poshuku
{
namespace DCAC
{
	EffectProcessor::EffectProcessor (QWidget *view)
	: QGraphicsEffect { view }
	{
	}

	void EffectProcessor::SetEffects (QList<Effect_t> effects)
	{
		if (effects == Effects_)
			return;

		Effects_ = std::move (effects);
		update ();
	}

	namespace
	{
		bool ApplyEffect (const Effect_t& effect, QImage& image)
		{
			return Util::Visit (effect,
					[&image] (const InvertEffect& effect)
					{
						return InvertColors (image, effect.Threshold_);
					},
					[&image] (const LightnessEffect& effect)
					{
						ReduceLightness (image, effect.Factor_);
						return true;
					},
					[&image] (const ColorTempEffect& effect)
					{
						AdjustColorTemp (image, effect.Temperature_);
						return true;
					});
		}
	}

	void EffectProcessor::draw (QPainter *painter)
	{
		if (Effects_.isEmpty ())
		{
			drawSource (painter);
			return;
		}

		QPoint offset;

		auto image = sourcePixmap (Qt::LogicalCoordinates, &offset, QGraphicsEffect::NoPad).toImage ();
		switch (image.format ())
		{
		case QImage::Format_ARGB32:
		case QImage::Format_ARGB32_Premultiplied:
			break;
		default:
			image = image.convertToFormat (QImage::Format_ARGB32);
			break;
		}
		image.detach ();

		bool hadEffects = false;
		for (const auto& effect : Effects_)
			if (ApplyEffect (effect, image))
				hadEffects = true;

		if (hadEffects)
			painter->drawImage (offset, image);
		else
			drawSource (painter);
	}
}
}
}
