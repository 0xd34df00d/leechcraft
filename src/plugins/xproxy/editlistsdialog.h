/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "ui_editlistsdialog.h"

class QStandardItemModel;

namespace LC
{
namespace XProxy
{
	class ScriptsManager;
	class UrlListScript;

	class EditListsDialog : public QDialog
	{
		Q_OBJECT

		Ui::EditListsDialog Ui_;

		QStandardItemModel * const Model_;

		const ScriptsManager * const Manager_;
		QList<UrlListScript*> Scripts_;
	public:
		EditListsDialog (const QList<UrlListScript*>&, const ScriptsManager*, QWidget* = nullptr);

		const QList<UrlListScript*>& GetScripts () const;
	private slots:
		void on_AddButton__released ();
		void on_RemoveButton__released ();
	};
}
}
