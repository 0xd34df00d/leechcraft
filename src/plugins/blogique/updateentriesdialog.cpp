/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "updateentriesdialog.h"
#include "xmlsettingsmanager.h"

namespace LC
{
namespace Blogique
{
	UpdateEntriesDialog::UpdateEntriesDialog (QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);

		Ui_.EntriesCount_->setValue (XmlSettingsManager::Instance ()
				.Property ("LastLocalEntriesToView", 20).toInt ());
	}

	int UpdateEntriesDialog::GetCount () const
	{
		return  Ui_.EntriesCount_->value ();
	}

	void UpdateEntriesDialog::accept ()
	{
		XmlSettingsManager::Instance ()
				.setProperty ("LocalLoadAsk", !Ui_.UpdateAsk_->isChecked ());
		XmlSettingsManager::Instance ()
				.setProperty ("LastLocalEntriesToView", Ui_.EntriesCount_->value ());
		QDialog::accept ();
	}

}
}
