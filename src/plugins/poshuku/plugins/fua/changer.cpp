/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "changer.h"
#include <QPushButton>
#include <util/sll/prelude.h>

namespace LC
{
namespace Poshuku
{
namespace Fua
{
	Changer::Changer (const QList<QPair<QString, QString>>& ids,
			const QMap<QString, QString>& backLookup,
			const QString& suggestedDomain,
			const QString& selectedID,
			QWidget *parent)
	: QDialog (parent)
	, IDs_ (ids)
	, BackLookup_ (backLookup)
	{
		Ui_.setupUi (this);

		Ui_.Agent_->addItems (Util::Map (ids, [] (const QPair<QString, QString>& pair) { return pair.first; }));
		Ui_.Domain_->setText (suggestedDomain);
		Ui_.IDString_->setText (selectedID);
		Ui_.Agent_->setCurrentIndex (Ui_.Agent_->findText (BackLookup_ [selectedID]));
		SetEnabled ();
	}

	QString Changer::GetDomain () const
	{
		return Ui_.Domain_->text ();
	}

	QString Changer::GetID () const
	{
		return Ui_.IDString_->text ();
	}

	void Changer::on_Domain__textChanged ()
	{
		SetEnabled ();
	}

	void Changer::on_IDString__textChanged ()
	{
		SetEnabled ();
	}

	void Changer::on_Agent__currentIndexChanged (int idx)
	{
		if (idx >= 0)
			Ui_.IDString_->setText (IDs_.value (idx).second);
	}

	void Changer::SetEnabled ()
	{
		Ui_.ButtonBox_->button (QDialogButtonBox::Ok)->
			setEnabled (Ui_.Domain_->text ().size () &&
					Ui_.IDString_->text ().size ());
	}
}
}
}
