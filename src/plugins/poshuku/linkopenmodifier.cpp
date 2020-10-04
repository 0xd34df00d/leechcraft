/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "linkopenmodifier.h"
#include <QMouseEvent>
#include <QWidget>
#include <util/sll/lambdaeventfilter.h>

namespace LC
{
namespace Poshuku
{
	void LinkOpenModifier::InstallOn (QWidget *view)
	{
		const auto ef = Util::MakeLambdaEventFilter ([this] (QMouseEvent *e)
				{
					if (e->type () == QEvent::MouseButtonPress)
					{
						MouseButtons_ = e->buttons ();
						Modifiers_ = e->modifiers ();
					}

					return false;
				},
				this);
		view->installEventFilter (ef);
	}

	auto LinkOpenModifier::GetOpenBehaviourSuggestion () const -> OpenBehaviourSuggestion
	{
		if (!(MouseButtons_ == Qt::MiddleButton ||
				Modifiers_ & Qt::ControlModifier))
			return {};

		return { true, static_cast<bool> (Modifiers_ & Qt::ShiftModifier) };
	}

	void LinkOpenModifier::ResetSuggestionState ()
	{
		MouseButtons_ = {};
		Modifiers_ = {};
	}
}
}
