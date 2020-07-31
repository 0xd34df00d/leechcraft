/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "ui_sendmessagedialog.h"

namespace LC
{
namespace Blogique
{
namespace Metida
{
	class LJAccount;
	class LJProfile;
	
	class SendMessageDialog : public QDialog
	{
		Q_OBJECT

		Ui::SendMessageDialog Ui_;
		LJAccount *Account_;
		LJProfile *Profile_;

	public:
		explicit SendMessageDialog (LJProfile *profile, QWidget *parent = 0);
		void accept ();
		
		QStringList GetAddresses () const;
		void SetAddresses (const QStringList& addresses);
		QString GetSubject () const;
		QString GetText () const;
	};
}
}
}
