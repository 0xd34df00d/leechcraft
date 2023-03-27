/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "fontsizescrollchanger.h"
#include <QWidget>
#include <QWheelEvent>
#include <util/sll/lambdaeventfilter.h>
#include <util/sll/visitor.h>

namespace LC::Util
{
	namespace
	{
		auto AddSteps (int size, int steps)
		{
			constexpr auto minFontSize = 6;
			return std::max (minFontSize, size + steps);
		}

		auto AddSteps (QFont font, int steps)
		{
			if (const auto pts = font.pointSize (); pts > 0)
				font.setPointSize (AddSteps (pts, steps));
			else if (const auto pxs = font.pixelSize (); pxs > 0)
				font.setPixelSize (AddSteps (pxs, steps));
			return font;
		}
	}

	void InstallFontSizeChanger (QWidget& widget, const FontSizeChangerParams& params)
	{
		widget.installEventFilter (Util::MakeLambdaEventFilter<QEvent::Wheel> ([params] (QWheelEvent *e)
				{
					if (!(e->modifiers () & Qt::ControlModifier))
						return false;

					constexpr qreal degreesPerDelta = 1 / 8.;
					constexpr qreal degreesPerStep = 15.;

					auto degrees = e->angleDelta ().y () * degreesPerDelta;
					int steps = static_cast<int> (degrees / degreesPerStep);

					Visit (params,
							[=] (const auto& methods)
							{
								const auto newFont = AddSteps (methods.GetView_ (), steps);
								methods.SetView_ (newFont);
								if (e->modifiers () & Qt::ShiftModifier)
									methods.SetDefault_ (newFont);
							});

					return true;
				},
				widget));
	}
}
