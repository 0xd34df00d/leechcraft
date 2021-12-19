/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "ui_vcarddialog.h"

namespace LC::Azoth::Acetamide
{
	class EntryBase;
	class IrcAccount;
	struct WhoIsMessage;

	class VCardDialog : public QDialog
	{
		Q_OBJECT

		Ui::VCardDialog Ui_;
	public:
		explicit VCardDialog (QWidget *parent = nullptr);

		void UpdateInfo (const WhoIsMessage& msg);
	};
}
