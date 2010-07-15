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

#include "glooxaccountconfigurationdialog.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Azoth
		{
			namespace Plugins
			{
				namespace Xoox
				{
					GlooxAccountConfigurationDialog::GlooxAccountConfigurationDialog (QWidget *parent)
					: QDialog (parent)
					{
						Ui_.setupUi (this);
					}

					QString GlooxAccountConfigurationDialog::GetJID () const
					{
						return Ui_.JID_->text ();
					}

					QString GlooxAccountConfigurationDialog::GetNick () const
					{
						return Ui_.Nick_->text ();
					}

					QString GlooxAccountConfigurationDialog::GetResource () const
					{
						return Ui_.Resource_->text ();
					}

					short GlooxAccountConfigurationDialog::GetPriority () const
					{
						return Ui_.Priority_->value ();
					}
				}
			}
		}
	}
}
