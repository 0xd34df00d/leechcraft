/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "ui_managerdialog.h"

class QSslCertificate;

namespace LC
{
namespace CertMgr
{
	class Manager;

	class ManagerDialog : public QDialog
	{
		Q_OBJECT

		Ui::ManagerDialog Ui_;
		Manager * const Manager_;

		enum class CertPart
		{
			System,
			Local
		};
	public:
		ManagerDialog (Manager*, QWidget* = 0);
	private:
		QSslCertificate GetSelectedCert (CertPart) const;
	private slots:
		void on_AddLocal__released ();
		void on_RemoveLocal__released ();
		void updateLocalButtons ();

		void on_Enable__released ();
		void on_Disable__released ();
		void updateSystemButtons ();
	};
}
}
