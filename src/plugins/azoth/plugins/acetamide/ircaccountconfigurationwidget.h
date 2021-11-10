/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011 Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include "ui_ircaccountconfigurationwidget.h"

namespace LC::Azoth::Acetamide
{
	class IrcAccountConfigurationWidget : public QWidget
	{
		Ui::IrcAccountConfigurationWidget Ui_;
	public:
		explicit IrcAccountConfigurationWidget (QWidget* = nullptr);

		void SetRealName (const QString&);
		QString GetRealName () const;
		void SetUserName (const QString&);
		QString GetUserName () const;
		void SetNickNames (const QStringList&);
		QStringList GetNickNames () const;
		void SetDefaultServer (const QString&);
		QString GetDefaultServer () const;
		void SetDefaultPort (int);
		int GetDefaultPort () const;
		void SetDefaultEncoding (const QString&);
		QString GetDefaultEncoding () const;
		void SetDefaultChannel (const QString&);
		QString GetDefaultChannel () const;
	};
}
