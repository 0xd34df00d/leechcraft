/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_PLUGINS_ACETAMIDE_NEWNICKSERVIDENTIFYDIALOG_H
#define PLUGINS_AZOTH_PLUGINS_ACETAMIDE_NEWNICKSERVIDENTIFYDIALOG_H

#include <QDialog>
#include "ui_newnickservidentifydialog.h"

namespace LC
{
namespace Azoth
{
namespace Acetamide
{
	struct NickServIdentify;

	class NewNickServIdentifyDialog : public QDialog
	{
		Q_OBJECT

		Ui::NewNickServDataDialog Ui_;
	public:
		explicit NewNickServIdentifyDialog (QWidget* = nullptr);

		NickServIdentify GetIdentify () const;
		void SetIdentify (const NickServIdentify&);
	};
}
}
}

#endif // PLUGINS_AZOTH_PLUGINS_ACETAMIDE_NEWNICKSERVIDENTIFYDIALOG_H
