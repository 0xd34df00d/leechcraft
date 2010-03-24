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

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Summary
		{
			SearchWidget::SearchWidget (QWidget *parent)
			: QDockWidget (parent)
			{
				Ui_.setupUi (this);
				Ui_.LeastCategory_->setDuplicatesEnabled (true);
			}

			QComboBox* SearchWidget::GetLeastCategory () const
			{
				return Ui_.LeastCategory_;
			}

			QLineEdit* SearchWidget::GetFilterLine () const
			{
				return Ui_.FilterLine_;
			}

			QComboBox* SearchWidget::GetFilterType () const
			{
				return Ui_.Type_;
			}

			void SearchWidget::AddCategory (QComboBox *box)
			{
				Ui_.SearchStuff_->insertWidget (1, box);
			}
		};
	};
};

