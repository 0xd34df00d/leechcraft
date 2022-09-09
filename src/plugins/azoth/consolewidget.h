/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QPointer>
#include "interfaces/azoth/ihaveconsole.h"
#include "tabbase.h"
#include "ui_consolewidget.h"

namespace LC
{
namespace Azoth
{
	class IAccount;
	class IHaveConsole;

	class ConsoleWidget : public TabBase
	{
		Q_OBJECT

		Ui::ConsoleWidget Ui_;

		QPointer<QObject> AsObject_;
		IAccount *AsAccount_;
		IHaveConsole *AsConsole_;
		const IHaveConsole::PacketFormat Format_;
	public:
		ConsoleWidget (QObject*, QWidget* = 0);

		void Remove () override;
		QToolBar* GetToolBar () const override;

		QString GetTitle () const;
	private slots:
		void handleConsolePacket (QByteArray, IHaveConsole::PacketDirection, const QString&);
	};
}
}
