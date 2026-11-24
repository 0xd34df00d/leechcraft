/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include <QList>
#include <QSslError>
#include "ui_sslerrorsdialog.h"

namespace LC
{
	class SslErrorsDialog : public QDialog
	{
		Q_OBJECT

		Ui::SslErrorsDialog Ui_;
	public:
		SslErrorsDialog (const QString& host, const QList<QSslError>&, QWidget* = nullptr);

		bool IsPermanent () const;
	private:
		void PopulateTree (const QSslError&);
	};
}
