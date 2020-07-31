/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "addtometacontactsdialog.h"
#include <interfaces/azoth/iclentry.h>
#include "metaentry.h"

namespace LC
{
namespace Azoth
{
namespace Metacontacts
{
	AddToMetacontactsDialog::AddToMetacontactsDialog (ICLEntry *entry,
			const QList<MetaEntry*>& metas, QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);
		
		Ui_.EntryName_->setText (QString ("%1 (%2)")
				.arg (entry->GetEntryName ())
				.arg (entry->GetHumanReadableID ()));
		
		Ui_.NewMetaname_->setText (entry->GetEntryName ());
		
		for (const auto entry : metas)
			Ui_.ExistingMeta_->addItem (entry->GetEntryName (), QVariant::fromValue<QObject*> (entry));
	}
	
	MetaEntry* AddToMetacontactsDialog::GetSelectedMeta () const
	{
		QObject *obj = Ui_.ExistingMeta_->
				itemData (Ui_.ExistingMeta_->currentIndex ()).value<QObject*> ();
		return qobject_cast<MetaEntry*> (obj);
	}
	
	QString AddToMetacontactsDialog::GetNewMetaName () const
	{
		return Ui_.NewMetaname_->text ();
	}
	
	void AddToMetacontactsDialog::on_ExistingMeta__currentIndexChanged (int idx)
	{
		Ui_.NewMetaname_->setEnabled (!idx);
	}
}
}
}
