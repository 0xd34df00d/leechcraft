/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_PLUGINS_ACETAMIDE_VCARDDIALOG_H
#define PLUGINS_AZOTH_PLUGINS_ACETAMIDE_VCARDDIALOG_H

#include <QDialog>
#include "localtypes.h"
#include "ui_vcarddialog.h"

namespace LC
{
namespace Azoth
{
namespace Acetamide
{
	class EntryBase;
	class IrcAccount;

	class VCardDialog : public QDialog
	{
		Q_OBJECT

		Ui::VCardDialog Ui_;
	public:
		VCardDialog (QWidget *parent = 0);

		void UpdateInfo (const WhoIsMessage& msg);
	};
}
}
}

#endif
