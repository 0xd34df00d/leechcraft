/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011 Oleg Linkin
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

#ifndef PLUGINS_AZOTH_PLUGINS_ACETAMIDE_INVITECHANNELSDIALOG_H
#define PLUGINS_AZOTH_PLUGINS_ACETAMIDE_INVITECHANNELSDIALOG_H

#include <QDialog>
#include "ui_invitechannelsdialog.h"

class QStandardItemModel;

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	class InviteChannelsDialog : public QDialog
	{
		Q_OBJECT
		Ui::InviteChannelsDialog Ui_;
		QStandardItemModel *Model_;
		QStandardItemModel *ActionsModel_;
		enum ActionByDefault
		{
			Ask,
			JoinAll,
			IgnoreAll
		};

		enum ActionsRole
		{
			ActionRole = Qt::UserRole + 1
		};
	public:
		InviteChannelsDialog (const QString&,
				const QString&, QWidget* = 0);
		void AddInvitation (const QString&, const QString&);
		QStringList GetChannels () const;
	public slots:
		void accept ();
	};
};
};
};

#endif // PLUGINS_AZOTH_PLUGINS_ACETAMIDE_INVITECHANNELSDIALOG_H
