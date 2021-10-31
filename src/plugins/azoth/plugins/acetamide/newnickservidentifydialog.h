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
	class NewNickServIdentifyDialog : public QDialog
	{
		Q_OBJECT

		Ui::NewNickServDataDialog Ui_;
	public:
		NewNickServIdentifyDialog (QWidget* = 0);

		QString GetServer () const;
		QString GetNickName () const;
		QString GetNickServNickName () const;
		QString GetAuthString () const;
		QString GetAuthMessage () const;
		void SetServer (const QString&);
		void SetNickName (const QString&);
		void SetNickServNickName (const QString&);
		void SetAuthString (const QString&);
		void SetAuthMessage (const QString&);
	public slots:
		void accept ();
	};
}
}
}

#endif // PLUGINS_AZOTH_PLUGINS_ACETAMIDE_NEWNICKSERVIDENTIFYDIALOG_H
