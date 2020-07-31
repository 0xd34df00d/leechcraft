/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011 Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCACCOUNTCONFIGURATIONWIDGET_H
#define PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCACCOUNTCONFIGURATIONWIDGET_H

#include <QWidget>
#include "ui_ircaccountconfigurationwidget.h"

namespace LC
{
namespace Azoth
{
namespace Acetamide
{
	class IrcAccountConfigurationWidget : public QWidget
	{
		Q_OBJECT

		Ui::IrcAccountConfigurationWidget Ui_;
	public:
		IrcAccountConfigurationWidget (QWidget* = 0);

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
};
};
};

#endif // PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCACCOUNTCONFIGURATIONWIDGET_H
