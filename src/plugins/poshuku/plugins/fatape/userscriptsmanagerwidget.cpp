/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "userscriptsmanagerwidget.h"
#include <QDebug>
#include <QStandardItemModel>
#include "fatape.h"

namespace LeechCraft
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
		connect (Ui_.Items_->selectionModel (),
				SIGNAL (currentChanged (const QModelIndex&, const QModelIndex&)),
				this,
				SLOT (currentItemChanged (const QModelIndex&, const QModelIndex&)));
	}

	void UserScriptsManagerWidget::on_Edit__released ()
	{
		const QModelIndex& selected = Ui_.Items_->currentIndex ();

		if (selected.isValid ())
			Plugin_->EditScript (selected.row ());
	}

	void UserScriptsManagerWidget::on_Disable__released ()
	{
		const QModelIndex& selected = Ui_.Items_->currentIndex ();

		if (!selected.isValid ())
			return;

		QStandardItemModel *model = qobject_cast<QStandardItemModel*> (Ui_.Items_->model ());


		if (!model)
		{
			qWarning () << Q_FUNC_INFO
				<< "unable cast "
				<< Ui_.Items_->model ()
				<< "to QStandardItemModel";
			return;
		}

		bool enabled = selected.data (EnabledRole).toBool ();

		Plugin_->SetScriptEnabled (selected.row (), !enabled);

		for (int column = 0; column < model->columnCount (); column++)
			model->item (selected.row (), column)->setData (!enabled, EnabledRole);

		Ui_.Disable_->setText (!enabled ? tr ("Disable") : tr ("Enable"));
	}

	void UserScriptsManagerWidget::on_Remove__released ()
	{
		const QModelIndex& selected = Ui_.Items_->currentIndex ();

		if (selected.isValid ())
		{
			Ui_.Items_->model ()->removeRow (selected.row ());
			Plugin_->DeleteScript (selected.row ());
		}
	}

	void UserScriptsManagerWidget::currentItemChanged (const QModelIndex& current,
			const QModelIndex& previous)
	{
		bool currentEnabled = current.data (EnabledRole).toBool ();
		bool previousEnabled = previous.data (EnabledRole).toBool ();

		if ((previous.isValid () && currentEnabled != previousEnabled) || !previous.isValid ())
			Ui_.Disable_->setText (currentEnabled ? tr ("Disable") : tr ("Enable"));
	}
}
}
}

