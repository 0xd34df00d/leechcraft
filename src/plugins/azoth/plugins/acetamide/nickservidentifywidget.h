/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include "ui_nickservidentifywidget.h"

namespace LC::Util
{
	class FlatItemsModelBase;
}

namespace LC
{
namespace Azoth
{
namespace Acetamide
{
	struct NickServIdentify;

	class NickServIdentifyWidget : public QWidget
	{
		Q_OBJECT

		enum Column
		{
			ServerName,
			Nick,
			NickServ,
			AuthString,
			AuthMessage
		};

		Ui::NickServIdentifyWidget Ui_;
	public:
		explicit NickServIdentifyWidget (Util::FlatItemsModelBase&, QWidget* = nullptr);
	public slots:
		void accept ();
	private slots:
		void on_Add__clicked ();
		void on_Edit__clicked ();
		void on_Delete__clicked ();
	signals:
		void saveSettings ();

		void identifyAdded (const NickServIdentify&);
		void identifyEdited (int, const NickServIdentify&);
		void identifyRemoved (int);
	};
}
}
}
