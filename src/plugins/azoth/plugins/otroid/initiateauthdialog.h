/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "ui_initiateauthdialog.h"

namespace LC
{
namespace Azoth
{
class ICLEntry;

namespace OTRoid
{
	enum class SmpMethod;

	class InitiateAuthDialog : public QDialog
	{
		Q_OBJECT

		Ui::InitiateAuthDialog Ui_;
	public:
		InitiateAuthDialog (ICLEntry*);

		SmpMethod GetMethod () const;
		QString GetQuestion () const;
		QString GetAnswer () const;
	private slots:
		void on_MethodBox__currentIndexChanged ();
	};
}
}
}
