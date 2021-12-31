/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "aalabeleventfilter.h"
#include <QMouseEvent>
#include "util.h"

namespace LC::LMP
{
	AALabelEventFilter::AALabelEventFilter (CoverPathGetter_t getter, QObject *parent)
	: QObject { parent }
	, Getter_ { std::move (getter) }
	{
	}

	bool AALabelEventFilter::eventFilter (QObject*, QEvent *event)
	{
		if (event->type () != QEvent::MouseButtonRelease)
			return false;

		ShowAlbumArt (Getter_ (), static_cast<QMouseEvent*> (event)->pos ());
		return true;
	}
}
