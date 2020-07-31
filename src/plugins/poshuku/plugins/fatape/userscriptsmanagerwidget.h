/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QStandardItemModel>
#include <QList>
#include <QWidget>
#include "ui_userscriptsmanagerwidget.h"

namespace LC
{
namespace Poshuku
{
namespace FatApe
{
	class Plugin;

	class UserScriptsManagerWidget : public QWidget
	{
		Q_OBJECT

		Ui::UserScriptsManagerWidget Ui_;
		QStandardItemModel * const Model_;
		Plugin * const Plugin_;
	public:
		UserScriptsManagerWidget (QStandardItemModel *model, Plugin *plugin);
	public slots:
		void on_Edit__released ();
		void on_Remove__released ();
		void on_Create__released ();
	};
}
}
}
