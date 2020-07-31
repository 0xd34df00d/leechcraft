/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "userscriptsmanagerwidget.h"
#include <QDebug>
#include <QStandardItemModel>
#include "fatape.h"
#include "userscriptcreator.h"

namespace LC
{
namespace Poshuku
{
namespace FatApe
{
	UserScriptsManagerWidget::UserScriptsManagerWidget (QStandardItemModel *model, Plugin *plugin)
	: Model_ (model)
	, Plugin_ (plugin)
	{
		Ui_.setupUi (this);
		Ui_.Items_->setModel (model);
	}

	void UserScriptsManagerWidget::on_Edit__released ()
	{
		const auto& selected = Ui_.Items_->currentIndex ();

		if (selected.isValid ())
			Plugin_->EditScript (selected.row ());
	}

	void UserScriptsManagerWidget::on_Remove__released ()
	{
		const auto& selected = Ui_.Items_->currentIndex ();

		if (selected.isValid ())
		{
			Ui_.Items_->model ()->removeRow (selected.row ());
			Plugin_->DeleteScript (selected.row ());
		}
	}

	void UserScriptsManagerWidget::on_Create__released ()
	{
		new UserScriptCreator { Plugin_, this };
	}
}
}
}
