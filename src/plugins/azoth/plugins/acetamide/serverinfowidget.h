/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include <interfaces/azoth/iconfigurablemuc.h>
#include "ui_serverinfowidget.h"

namespace LC::Azoth::Acetamide
{
	class IrcServerCLEntry;

	class ServerInfoWidget : public QWidget
							, public IMUCConfigWidget
	{
		Q_OBJECT
		Q_INTERFACES (LC::Azoth::IMUCConfigWidget)

		Ui::ServerInfoWidget Ui_;
		IrcServerCLEntry * const ISCLEntry_;
	public:
		explicit ServerInfoWidget (IrcServerCLEntry*, QWidget* = nullptr);
	private:
		void HandleSpecial (QHash<QByteArray, QVariant>&);

		void SetPrefix (const QByteArray&);
		void SetTargMax (const QByteArray&);
	public slots:
		void accept ();
	signals:
		void dataReady ();
	};
}
