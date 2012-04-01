/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#ifndef PLUGINS_TABSESSMANAGER_RESTORESESSIONDIALOG_H
#define PLUGINS_TABSESSMANAGER_RESTORESESSIONDIALOG_H
#include <QDialog>
#include "ui_restoresessiondialog.h"
#include "recinfo.h"

namespace LeechCraft
{
namespace TabSessManager
{
	class RestoreSessionDialog : public QDialog
	{
		Q_OBJECT

		Ui::RestoreSessionDialog Ui_;
	public:
		RestoreSessionDialog (QWidget* = 0);

		void SetPages (const QHash<QObject*, QList<RecInfo>>&);
		QHash<QObject*, QList<RecInfo>> GetPages () const;
	};
}
}

#endif
