/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_PLUGINS_ACETAMIDE_NICKSERVIDENTIFYWIDGET_H
#define PLUGINS_AZOTH_PLUGINS_ACETAMIDE_NICKSERVIDENTIFYWIDGET_H

#include <QWidget>
#include "ui_nickservidentifywidget.h"

class QStandardItemModel;

namespace LC
{
namespace Azoth
{
namespace Acetamide
{
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
		QStandardItemModel* Model_;
	public:
		NickServIdentifyWidget (QStandardItemModel*, QWidget* = 0);
	private:
		void ReadSettings ();
	public slots:
		void accept ();
	private slots:
		void on_Add__clicked ();
		void on_Edit__clicked ();
		void on_Delete__clicked ();
	};
};
};
};

#endif // PLUGINS_AZOTH_PLUGINS_ACETAMIDE_NICKSERVIDENTIFYWIDGET_H
