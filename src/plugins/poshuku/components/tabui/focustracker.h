/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include "interfaces/poshuku/iwebview.h"

class QLineEdit;

namespace LC::Poshuku
{
	class FocusTracker : public QObject
	{
		Q_OBJECT

		QLineEdit& UrlEdit_;
		QWidget& ViewWidget_;

		bool FocusOnLoad_ = true;
	public:
		FocusTracker (QLineEdit& urlEdit, const IWebView_ptr& view, QWidget& tab);
	private:
		void FocusUrlLine ();
	private slots:
		void handleLoadFinished ();
	};
}
