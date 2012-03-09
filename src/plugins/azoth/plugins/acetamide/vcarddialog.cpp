/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
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

#include "vcarddialog.h"
#include <QPushButton>
#include <QFileDialog>
#include <QBuffer>
#include "entrybase.h"
#include "ircaccount.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	VCardDialog::VCardDialog (EntryBase *entry, QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);
	}

	void VCardDialog::UpdateInfo (const WhoIsMessage& msg)
	{
		setWindowTitle (tr ("VCard for %1").arg (msg.Nick_));

		Ui_.EditNick_->setText (msg.Nick_);
		Ui_.EditUserName_->setText (msg.UserName_);
		Ui_.EditHostName_->setText (msg.Host_);
		Ui_.EditRealName_->setText (msg.RealName_);
		Ui_.ServerName_->setText (msg.ServerName_);
		Ui_.ServerDislocation_->setText (msg.ServerCountry_);
	}
}
}
}
