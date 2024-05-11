/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "rotatemenu.h"
#include <QCoreApplication>
#include <QMenu>
#include <QWidgetAction>
#include <util/sll/visitor.h>
#include "arbitraryrotationwidget.h"
#include "pageslayoutmanager.h"

namespace LC::Monocle
{
	namespace
	{
		struct Tr
		{
			Q_DECLARE_TR_FUNCTIONS (LC::Monocle::RotateMenuFactory)
		};
	}

	std::unique_ptr<QMenu> CreateRotateMenu (AngleGetter angleGetter,
			const std::function<void (double, RotationChange)>& angleSetter)
	{
		auto menu = std::make_unique<QMenu> ();

		constexpr auto SingleRotation = 90;

		auto ccwAction = menu->addAction (Tr::tr ("Rotate 90 degrees counter-clockwise"),
				[angleSetter] { angleSetter (-SingleRotation, RotationChange::Add); });
		ccwAction->setProperty ("ActionIcon", "object-rotate-left");

		auto cwAction = menu->addAction (Tr::tr ("Rotate 90 degrees clockwise"),
				[angleSetter] { angleSetter (+SingleRotation, RotationChange::Add); });
		cwAction->setProperty ("ActionIcon", "object-rotate-right");

		auto arbAction = menu->addAction (Tr::tr ("Rotate arbitrarily..."));
		arbAction->setProperty ("ActionIcon", "transform-rotate");

		auto arbMenu = new QMenu ();
		arbAction->setMenu (arbMenu);

		auto arbWidget = new ArbitraryRotationWidget;
		QObject::connect (arbWidget,
				&ArbitraryRotationWidget::valueChanged,
				[angleSetter] (double value) { angleSetter (value, RotationChange::Set); });
		Util::Visit (angleGetter,
				[arbWidget] (InitAngle angle) { arbWidget->setValue (angle.Value_); },
				[arbWidget]<typename T> (AngleNotifier<T> notifier)
				{
					QObject::connect (&notifier.Obj_,
							notifier.Signal_,
							arbWidget,
							&ArbitraryRotationWidget::setValue);
				});
		auto actionWidget = new QWidgetAction (menu.get ());
		actionWidget->setDefaultWidget (arbWidget);
		arbMenu->addAction (actionWidget);

		return menu;
	}
}
