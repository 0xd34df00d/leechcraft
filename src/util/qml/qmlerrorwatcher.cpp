/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "qmlerrorwatcher.h"
#include <QQuickWidget>
#include <QQmlError>
#include <QtDebug>
#include <util/sll/slotclosure.h>

namespace LC
{
namespace Util
{
	QmlErrorWatcher::QmlErrorWatcher (QQuickWidget *view)
	: QObject { view }
	{
		new Util::SlotClosure<Util::NoDeletePolicy>
		{
			[view]
			{
				if (view->status () == QQuickWidget::Error)
				{
					qWarning () << Q_FUNC_INFO
							<< "view errors:";
					for (const auto& err : view->errors ())
						qWarning () << "\t"
								<< err.toString ();
				}
			},
			view,
			SIGNAL (statusChanged (QQuickWidget::Status)),
			view
		};
	}
}
}
