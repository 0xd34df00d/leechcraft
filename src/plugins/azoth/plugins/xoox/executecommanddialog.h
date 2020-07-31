/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWizard>
#include "ui_executecommanddialog.h"
#include "xeps/adhoccommandmanager.h"

namespace LC
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
		AdHocCommandManager& Manager_;
		QString JID_;

		struct Tag {};
		ExecuteCommandDialog (const QString&, GlooxAccount*, QWidget*, Tag);
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

		void handleError (const QString&);

		void recreate ();
	};
}
}
}
