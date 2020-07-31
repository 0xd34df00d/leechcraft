/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_KNOWHOW_TIPDIALOG_H
#define PLUGINS_KNOWHOW_TIPDIALOG_H
#include <memory>
#include <QDialog>
#include <interfaces/core/icoreproxy.h>
#include "ui_tipdialog.h"

class QDomDocument;

namespace LC
{
namespace KnowHow
{
	class TipDialog : public QDialog
	{
		Q_OBJECT

		Ui::TipDialog Ui_;
		std::shared_ptr<QDomDocument> Doc_;
		ICoreProxy_ptr Proxy_;
	public:
		TipDialog (ICoreProxy_ptr, QWidget* = 0);
	private:
		void ShowForIdx (int);
		QString GetTipByID (int);
	private slots:
		void on_Forward__released ();
		void on_Backward__released ();
		void on_DontShow__stateChanged ();
	};
}
}

#endif
