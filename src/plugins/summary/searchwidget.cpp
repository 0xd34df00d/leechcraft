/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2009  Georg Rudoy
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

#include "searchwidget.h"
#include <plugininterface/categoryselector.h>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Summary
		{
			SearchWidget::SearchWidget (QWidget *parent)
			: QDockWidget (parent)
			, CategorySelector_ (new Util::CategorySelector ())
			{
				Ui_.setupUi (this);

				Ui_.SearchStuff_->addWidget (CategorySelector_);
				CategorySelector_->SetCaption (tr ("Search categories"));

				connect (CategorySelector_,
						SIGNAL (selectionChanged (const QStringList&)),
						this,
						SIGNAL (paramsChanged ()));

				connect (Ui_.Or_,
						SIGNAL (toggled (bool)),
						this,
						SIGNAL (paramsChanged ()));
			}

			QLineEdit* SearchWidget::GetFilterLine () const
			{
				return Ui_.FilterLine_;
			}

			QComboBox* SearchWidget::GetFilterType () const
			{
				return Ui_.Type_;
			}

			bool SearchWidget::IsOr () const
			{
				return Ui_.Or_->isChecked ();
			}

			QStringList SearchWidget::GetCategories () const
			{
				return CategorySelector_->GetSelections ();
			}

			void SearchWidget::SetPossibleCategories (const QStringList& possible)
			{
				CategorySelector_->SetPossibleSelections (possible);
			}

			void SearchWidget::SelectCategories (const QStringList& subset)
			{
				CategorySelector_->SetSelections (subset);
			}
		};
	};
};

