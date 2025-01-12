/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include <QSslError>
#include "ui_sslerrorsdialog.h"
#include "sslerrorshandler.h"

template<typename T>
class QList;

namespace LC
{
namespace Azoth
{
	class SslErrorsDialog : public QDialog
	{
		Q_DECLARE_TR_FUNCTIONS (SslErrorsDialog)

		Ui::SslErrorsDialog Ui_;
	public:
		SslErrorsDialog (const SslErrorsHandler::Context_t&, const QList<QSslError>&, QWidget* = nullptr);

		bool ShouldRememberChoice () const;
	};
}
}
