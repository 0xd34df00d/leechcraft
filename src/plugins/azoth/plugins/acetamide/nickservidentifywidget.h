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

namespace LC::Azoth::Acetamide
{
	struct NickServIdentify;

	class NickServIdentifyWidget : public QWidget
	{
		Q_OBJECT

		Ui::NickServIdentifyWidget Ui_;
		QList<QDialog*> DeleteInvalidatedDialogs_;
	public:
		explicit NickServIdentifyWidget (Util::FlatItemsModelBase&, QWidget* = nullptr);
	public slots:
		void accept ();
	signals:
		void saveSettings ();

		void identifyAdded (const NickServIdentify&);
		void identifyEdited (int, const NickServIdentify&);
		void identifyRemoved (int);
	};
}
