/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "ui_editurldialog.h"

namespace LC
{
namespace XProxy
{
	struct ReqTarget;

	class EditUrlDialog : public QDialog
	{
		Q_OBJECT

		Ui::EditUrlDialog Ui_;
	public:
		EditUrlDialog (QWidget* = nullptr);

		ReqTarget GetReqTarget () const;
		void SetReqTarget (const ReqTarget&);
	};
}
}
