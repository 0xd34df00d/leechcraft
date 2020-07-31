/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "pgpkeyselectiondialog.h"
#include "cryptomanager.h"

namespace LC
{
namespace Azoth
{
	PGPKeySelectionDialog::PGPKeySelectionDialog (const QString& label,
			PGPKeySelectionDialog::Type type,
			const QCA::PGPKey& focusKey,
			QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);
		Ui_.LabelText_->setText (label);

		switch (type)
		{
		case TPrivate:
			Keys_ = CryptoManager::Instance ().GetPrivateKeys ();
			break;
		case TPublic:
			Keys_ = CryptoManager::Instance ().GetPublicKeys ();
			break;
		}

		const auto& focusArr = !focusKey.isNull () ? focusKey.toArray () : QByteArray ();
		for (const auto& key : Keys_)
		{
			Ui_.KeyCombo_->addItem (key.primaryUserId () + " (" + key.keyId () + ")");
			if (key.toArray () == focusArr)
				Ui_.KeyCombo_->setCurrentIndex (Ui_.KeyCombo_->count () - 1);
		}
	}

	QCA::PGPKey PGPKeySelectionDialog::GetSelectedKey () const
	{
		const int idx = Ui_.KeyCombo_->currentIndex ();
		return idx > 0 ? Keys_ [idx - 1] : QCA::PGPKey ();
	}
}
}
