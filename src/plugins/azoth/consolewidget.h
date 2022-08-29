/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_CONSOLEWIDGET_H
#define PLUGINS_AZOTH_CONSOLEWIDGET_H
#include <QWidget>
#include <QPointer>
#include <interfaces/ihavetabs.h>
#include "interfaces/azoth/ihaveconsole.h"
#include "ui_consolewidget.h"

namespace LC
{
namespace Azoth
{
	class IAccount;
	class IHaveConsole;

	class ConsoleWidget : public QWidget
						, public ITabWidget
	{
		Q_OBJECT
		Q_INTERFACES (ITabWidget)

		Ui::ConsoleWidget Ui_;
		QObject *ParentMultiTabs_ = nullptr;
		TabClassInfo TabClass_;

		QPointer<QObject> AsObject_;
		IAccount *AsAccount_;
		IHaveConsole *AsConsole_;
		const IHaveConsole::PacketFormat Format_;
	public:
		ConsoleWidget (QObject*, QWidget* = 0);

		TabClassInfo GetTabClassInfo () const override;
		QObject* ParentMultiTabs () override;
		void Remove () override;
		QToolBar* GetToolBar () const override;

		void SetParentMultiTabs (QObject*);
		QString GetTitle () const;
	private slots:
		void handleConsolePacket (QByteArray, IHaveConsole::PacketDirection, const QString&);
	signals:
		void removeTab () override;
	};
}
}

#endif
