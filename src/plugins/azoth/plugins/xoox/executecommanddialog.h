/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_EXECUTECOMMANDDIALOG_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_EXECUTECOMMANDDIALOG_H
#include <QWizard>
#include "ui_executecommanddialog.h"
#include "adhoccommandmanager.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	class GlooxAccount;
	class AdHocCommandManager;

	class ExecuteCommandDialog : public QWizard
	{
		Q_OBJECT

		Ui::ExecuteCommandDialog Ui_;
		GlooxAccount *Account_;
		AdHocCommandManager *Manager_;
		QString JID_;
	public:
		ExecuteCommandDialog (const QString&, GlooxAccount*, QWidget* = 0);
		ExecuteCommandDialog (const QString&, const QString&, GlooxAccount*, QWidget* = 0);
	private:
		void RequestCommands ();
		void ExecuteCommand (const AdHocCommand&);
		void ProceedExecuting (const AdHocResult&, const QString&);
	private slots:
		void handleCurrentChanged (int);
		void handleGotCommands (const QString&, const QList<AdHocCommand>&);
		void handleGotResult (const QString&, const AdHocResult&);
	};
}
}
}

#endif
