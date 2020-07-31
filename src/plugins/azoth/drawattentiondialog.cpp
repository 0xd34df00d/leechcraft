/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "drawattentiondialog.h"

namespace LC
{
namespace Azoth
{
	DrawAttentionDialog::DrawAttentionDialog (const QStringList& resources, QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);
		
		if (resources.size () > 1)
		{
			Ui_.ResourceBox_->addItem (tr ("<all>"));
			Ui_.ResourceBox_->addItems (resources);
		}
		else
			Ui_.ResourceBox_->setEnabled (false);
	}
	
	QString DrawAttentionDialog::GetResource () const
	{
		if (!Ui_.ResourceBox_->count () ||
				!Ui_.ResourceBox_->currentIndex ())
			return QString ();
		
		return Ui_.ResourceBox_->currentText ();
	}
	
	QString DrawAttentionDialog::GetText () const
	{
		return Ui_.Text_->text ();
	}
}
}
