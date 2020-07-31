/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "ui_gwoptionsdialog.h"

class QXmppClient;

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	class RegFormHandlerWidget;

	class GWOptionsDialog : public QDialog
	{
		Q_OBJECT

		Ui::GWOptionsDialog Ui_;
		RegFormHandlerWidget *RegForm_;
	public:
		GWOptionsDialog (QXmppClient*, const QString& to, QWidget* = 0);
	private slots:
		void sendRegistration ();
		void handleError (const QString&);
		void handleCompleteChanged ();
	};
}
}
}
