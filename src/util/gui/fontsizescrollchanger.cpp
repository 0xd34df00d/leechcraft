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

namespace LC::Util
{
	void InstallFontSizeChanger (QWidget& widget, const FontSizeChangerParams& params)
	{
		widget.installEventFilter (Util::MakeLambdaEventFilter<QEvent::Wheel> ([params] (QWheelEvent *e)
				{
					if (!(e->modifiers () & Qt::ControlModifier))
						return false;

					int degrees = e->angleDelta ().y () / 8.;
					int steps = static_cast<qreal> (degrees) / 15;

					const auto fontSize = std::max (6, params.GetViewFontSize_ () + steps);

					params.SetViewFontSize_ (fontSize);
					if (e->modifiers () & Qt::ShiftModifier)
						params.SetDefaultFontSize_ (fontSize);

					return true;
				},
				widget));
	}
}
