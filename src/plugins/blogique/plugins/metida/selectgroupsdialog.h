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

#pragma once

#include <QDialog>
#include "ui_selectgroupsdialog.h"

class QStandardItemModel;

namespace LeechCraft
{
namespace Blogique
{
namespace Metida
{
	class LJProfile;

	class SelectGroupsDialog : public QDialog
	{
		Q_OBJECT

		Ui::SelectGroupsDialog Ui_;
		QStandardItemModel *Model_;
	public:
		SelectGroupsDialog (LJProfile *profile, quint32 allowMask, QWidget *parent = 0);

		QList<uint> GetSelectedGroupsIds () const;
	};
}
}
}

