/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include <QSettings>
#include <interfaces/core/icoreproxy.h>
#include "ui_acceptedrejecteddialog.h"

namespace LC
{
namespace CertMgr
{
	class ExceptionsModel;

	class AcceptedRejectedDialog : public QDialog
	{
		Q_OBJECT

		Ui::AcceptedRejectedDialog Ui_;

		const ICoreProxy_ptr Proxy_;

		QSettings CoreSettings_;
		ExceptionsModel * const Model_;
	public:
		AcceptedRejectedDialog (ICoreProxy_ptr);
		~AcceptedRejectedDialog ();
	private slots:
		void on_RemoveButton__released ();
		void handleSelectionChanged ();

		void toggleState (const QModelIndex&);
		void adjustWidths ();
	};
}
}
