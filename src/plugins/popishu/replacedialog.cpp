/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2010  Georg Rudoy
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

#include "replacedialog.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Popishu
		{
			ReplaceDialog::ReplaceDialog (QWidget *parent)
			: QDialog (parent)
			{
				Ui_.setupUi (this);
			}

			QString ReplaceDialog::GetBefore () const
			{
				return Ui_.Before_->text ();
			}

			QString ReplaceDialog::GetAfter () const
			{
				return Ui_.After_->text ();
			}

			Qt::CaseSensitivity ReplaceDialog::GetCaseSensitivity () const
			{
				return Ui_.CaseSensitive_->isChecked () ?
						Qt::CaseSensitive :
						Qt::CaseInsensitive;
			}

			ReplaceDialog::Scope ReplaceDialog::GetScope () const
			{
				if (Ui_.ScopeSelected_->isChecked ())
					return SSelected;
				else
					return SAll;
			}
		}
	}
}
