/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#include "addpeerdialog.h"
#include "ipvalidators.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace BitTorrent
		{
			AddPeerDialog::AddPeerDialog (QWidget *parent)
			: QDialog (parent)
			{
				Ui_.setupUi (this);
				Ui_.IP4_->setValidator (new ValidateIPv4 (this));
				Ui_.IP6_->setValidator (new ValidateIPv6 (this));
			}

			QString AddPeerDialog::GetIP () const
			{
				return Ui_.IP4_->isEnabled () ? Ui_.IP4_->text () : Ui_.IP6_->text ();
			}

			int AddPeerDialog::GetPort () const
			{
				return Ui_.Port_->value ();
			}
		};
	};
};

