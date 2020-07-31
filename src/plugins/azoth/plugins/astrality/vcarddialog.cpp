/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "vcarddialog.h"
#include <util/sll/prelude.h>

namespace LC
{
namespace Azoth
{
namespace Astrality
{
	VCardDialog::VCardDialog (QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);
	}

	void VCardDialog::SetAvatar (const QImage& image)
	{
		if (!image.isNull ())
			Ui_.AvatarLabel_->setPixmap (QPixmap::fromImage (image));
	}

	void VCardDialog::SetInfoFields (const Tp::ContactInfoFieldList& list)
	{
		if (list.isEmpty ())
		{
			Ui_.InfoFields_->setPlainText (tr ("No info or protocol doesn't support info."));
			return;
		}

		const auto& strings = Util::Map (list,
				[] (const auto& field) { return field.fieldName + ": " + field.fieldValue.join ("; "); });
		Ui_.InfoFields_->setPlainText (strings.join ("\n"));
	}
}
}
}
