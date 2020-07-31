/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "createscriptdialog.h"
#include <QPushButton>

namespace LC
{
namespace Poshuku
{
namespace FatApe
{
	CreateScriptDialog::CreateScriptDialog (QWidget *parent)
	: QDialog { parent }
	{
		Ui_.setupUi (this);

		checkValidity ();
		for (auto line : { Ui_.Namespace_, Ui_.Name_, Ui_.Description_, Ui_.Author_ })
			connect (line,
					SIGNAL (textChanged (QString)),
					this,
					SLOT (checkValidity ()));
	}

	QString CreateScriptDialog::GetNamespace () const
	{
		return Ui_.Namespace_->text ();
	}

	QString CreateScriptDialog::GetName () const
	{
		return Ui_.Name_->text ();
	}

	QString CreateScriptDialog::GetDescription () const
	{
		return Ui_.Description_->text ();
	}

	QString CreateScriptDialog::GetAuthor () const
	{
		return Ui_.Author_->text ();
	}

	void CreateScriptDialog::checkValidity ()
	{
		const bool hasInfo = !GetNamespace ().isEmpty () &&
				!GetName ().isEmpty ();
		Ui_.ButtonBox_->button (QDialogButtonBox::Ok)->setEnabled (hasInfo);
	}
}
}
}
