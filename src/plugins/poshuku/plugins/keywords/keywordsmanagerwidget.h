/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include <QStandardItemModel>
#include <QSettings>
#include "ui_keywordsmanagerwidget.h"

namespace LC
{
namespace Poshuku
{
namespace Keywords
{
	class Plugin;

	class KeywordsManagerWidget : public QWidget
	{
		Q_OBJECT

		Ui::KeywordsManagerWidget Ui_;
		QStandardItemModel *Model_; 
		Plugin *Plugin_;
		QSettings Keywords_;
	public:
		KeywordsManagerWidget (QStandardItemModel *model, Plugin *plugin);
	public slots:
		void on_Add__released ();
		void on_Modify__released ();
		void on_Remove__released ();
	};
}
}
}
