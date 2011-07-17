/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "addtometacontactsdialog.h"
#include <interfaces/iclentry.h>
#include "metaentry.h"

namespace LeechCraft
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
		
		Q_FOREACH (MetaEntry *entry, metas)
			Ui_.ExistingMeta_->addItem (entry->GetEntryName (),
					QVariant::fromValue<QObject*> (entry));
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
