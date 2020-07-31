/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_POSHUKU_HIDEABLETABWIDGET_H
#define PLUGINS_POSHUKU_HIDEABLETABWIDGET_H
#include <QWidget>
#include "ui_hideabletabwidget.h"

class QAction;
class QTabBar;

namespace LC
{
namespace Poshuku
{
	class HideableTabWidget : public QWidget
	{
		Q_OBJECT

		Ui::HideableTabWidget Ui_;

		QTabBar *TabBar_;
	public:
		HideableTabWidget (QWidget* = 0);
		void AddPage (const QString&, QWidget*);
		QTabBar* GetMainTabBar () const;
	};
}
}

#endif
