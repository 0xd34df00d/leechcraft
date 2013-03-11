/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#include "pgpkeyselectiondialog.h"
#include "core.h"

namespace LeechCraft
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
			Keys_ = Core::Instance ().GetPrivateKeys ();
			break;
		case TPublic:
			Keys_ = Core::Instance ().GetPublicKeys ();
			break;
		}

		const auto& focusArr = !focusKey.isNull () ? focusKey.toArray () : QByteArray ();
		Q_FOREACH (const QCA::PGPKey& key, Keys_)
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
