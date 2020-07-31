/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "radiocustomdialog.h"
#include <QUrl>

namespace LC
{
namespace LMP
{
	RadioCustomDialog::RadioCustomDialog (QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);
	}

	QUrl RadioCustomDialog::GetUrl () const
	{
		return QUrl::fromUserInput (Ui_.URL_->text ());
	}

	void RadioCustomDialog::SetUrl (const QUrl& url)
	{
		Ui_.URL_->setText (url.toString ());
	}

	QString RadioCustomDialog::GetName () const
	{
		return Ui_.Name_->text ();
	}

	void RadioCustomDialog::SetName (const QString& name)
	{
		Ui_.Name_->setText (name);
	}
}
}
