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

#ifndef PLUGINS_AZOTH_PLUGINS_ACETAMIDE_VCARDDIALOG_H
#define PLUGINS_AZOTH_PLUGINS_ACETAMIDE_VCARDDIALOG_H

#include <QDialog>
#include "localtypes.h"
#include "ui_vcarddialog.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	class EntryBase;
	class IrcAccount;

	class VCardDialog : public QDialog
	{
		Q_OBJECT

		Ui::VCardDialog Ui_;

		IrcAccount *Account_;
		QString ID_;
	public:
		VCardDialog (EntryBase *entry, QWidget *parent = 0);

		void UpdateInfo (const WhoIsMessage& msg);
	};
}
}
}

#endif
