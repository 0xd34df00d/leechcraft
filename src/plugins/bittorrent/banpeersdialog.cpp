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

#include "banpeersdialog.h"
#include "ipvalidators.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace BitTorrent
		{
			BanPeersDialog::BanPeersDialog (QWidget *parent)
			: QDialog (parent)
			{
				Ui_.setupUi (this);
				Ui_.IP4Start_->setValidator (new ValidateIPv4 (this));
				Ui_.IP4End_->setValidator (new ValidateIPv4 (this));
				Ui_.IP6Start_->setValidator (new ValidateIPv6 (this));
				Ui_.IP6End_->setValidator (new ValidateIPv6 (this));
			}

			void BanPeersDialog::SetIP (const QString& ip)
			{
				if (ip.contains (":"))
				{
					Ui_.RadioIP6_->setChecked (true);
					Ui_.IP6Start_->setText (ip);
					Ui_.IP6End_->setText (ip);
				}
				else
				{
					Ui_.IP4Start_->setText (ip);
					Ui_.IP4End_->setText (ip);
				}
			}

			void BanPeersDialog::SetIP (const QString& first, const QString& last)
			{
				if (first.contains (":"))
				{
					Ui_.RadioIP6_->setChecked (true);
					Ui_.IP6Start_->setText (first);
					Ui_.IP6End_->setText (last);
				}
				else
				{
					Ui_.IP4Start_->setText (first);
					Ui_.IP4End_->setText (last);
				}
			}

			QString BanPeersDialog::GetStart () const
			{
				return Ui_.IP4Start_->isEnabled () ? Ui_.IP4Start_->text () : Ui_.IP6Start_->text ();
			}

			QString BanPeersDialog::GetEnd () const
			{
				return Ui_.IP4Start_->isEnabled () ? Ui_.IP4End_->text () : Ui_.IP6End_->text ();
			}
		};
	};
};

