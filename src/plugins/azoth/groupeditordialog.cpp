/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010  Georg Rudoy
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

#include "groupeditordialog.h"
#include <QStringListModel>
#include <plugininterface/tagscompleter.h>
#include "core.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Azoth
		{
			GroupEditorDialog::GroupEditorDialog (const QStringList& initial,
					const QStringList& allGroups,
					QWidget *parent)
			: QDialog (parent)
			{
				Ui_.setupUi (this);

				Ui_.GroupsSelector_->setWindowFlags (Qt::Widget);
				Ui_.GroupsSelector_->SetPossibleSelections (allGroups);
				Ui_.GroupsSelector_->SetSelections (initial);

				Util::TagsCompleter *tc = new Util::TagsCompleter (Ui_.CategoriesLineEdit_);
				tc->OverrideModel (new QStringListModel (allGroups, this));

				const QString& text = Core::Instance ()
						.GetProxy ()->GetTagsManager ()->Join (initial);
				Ui_.CategoriesLineEdit_->setText (text);
				Ui_.CategoriesLineEdit_->AddSelector ();

				connect (Ui_.CategoriesLineEdit_,
						SIGNAL (textChanged (const QString&)),
						Ui_.GroupsSelector_,
						SLOT (lineTextChanged (const QString&)));
			}

			QStringList GroupEditorDialog::GetGroups () const
			{
				const QString& text = Ui_.CategoriesLineEdit_->text ();
				return Core::Instance ().GetProxy ()->
						GetTagsManager ()->Split (text);
			}

			void GroupEditorDialog::on_GroupsSelector__selectionChanged (const QStringList& groups)
			{
				const QString& text = Core::Instance ()
						.GetProxy ()->GetTagsManager ()->Join (groups);
				Ui_.CategoriesLineEdit_->setText (text);
			}
		}
	}
}
