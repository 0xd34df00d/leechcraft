/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "util.h"
#include <QQuickWidget>
#include <QQmlError>
#include <QtDebug>
#include <util/gui/geometry.h>

namespace LC::Util
{
	void EnableTransparency (QQuickWidget& widget)
	{
		widget.setAttribute (Qt::WA_TranslucentBackground);
		widget.setClearColor (Qt::transparent);
	}

	void SetupFullscreenView (QQuickWidget& widget)
	{
		widget.setWindowFlags (Qt::Dialog | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);

		const auto& rect = ScreenGeometry (QCursor::pos ());
		widget.setGeometry (rect);
		widget.setFixedSize (rect.size ());

		widget.setResizeMode (QQuickWidget::SizeRootObjectToView);
	}

	void WatchQmlErrors (QQuickWidget& view)
	{
		QObject::connect (&view,
				&QQuickWidget::statusChanged,
				[&view]
				{
					if (view.status () == QQuickWidget::Error)
					{
						qWarning () << Q_FUNC_INFO
								<< "view errors:";
						for (const auto& err : view.errors ())
							qWarning () << "\t"
									<< err.toString ();
					}
				});
	}
}
