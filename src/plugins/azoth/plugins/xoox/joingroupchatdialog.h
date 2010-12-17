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

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_JOINGROUPCHATDIALOG_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_JOINGROUPCHATDIALOG_H
#include <QDialog>
#include "ui_glooxjoingroupchatdialog.h"

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
	class GlooxAccount;

	class JoinGroupchatDialog : public QDialog
	{
		Q_OBJECT

		Ui::GlooxJoinGroupchatDialog Ui_;
		QList<GlooxAccount*> Accounts_;
	public:
		JoinGroupchatDialog (const QList<GlooxAccount*>&, QWidget* = 0);

		QString GetServer () const;
		QString GetRoom () const;
		QString GetNickname () const;
		GlooxAccount* GetSelectedAccount () const;
	private slots:
		void on_Account__currentIndexChanged (int);
	};
}
}
}
}
}

#endif
