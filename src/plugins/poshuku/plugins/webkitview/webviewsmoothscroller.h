/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>

class QWebView;
class QTimer;

namespace LC
{
namespace Poshuku
{
namespace WebKitView
{
	class WebViewSmoothScroller : public QObject
	{
		Q_OBJECT

		QWebView * const View_;
		QTimer * const ScrollTimer_;
		double ScrollDelta_ = 0;
		double AccumulatedScrollShift_ = 0;
	public:
		WebViewSmoothScroller (QWebView*);
	private slots:
		void handleAutoscroll ();
	};
}
}
}
