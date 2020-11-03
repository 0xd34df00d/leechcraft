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

namespace LC::Util
{
	void WatchQmlErrors (QQuickWidget *view)
	{
		QObject::connect (view,
				&QQuickWidget::statusChanged,
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
				});
	}
}
