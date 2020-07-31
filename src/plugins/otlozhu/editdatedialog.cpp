/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "editdatedialog.h"

namespace LC
{
namespace Otlozhu
{
	EditDateDialog::EditDateDialog (const QDateTime& initial, QWidget *parent)
	: QDialog { parent }
	{
		Ui_.setupUi (this);

		if (initial.isValid ())
		{
			Ui_.DateEdit_->setSelectedDate (initial.date ());
			Ui_.TimeEdit_->setTime (initial.time ());
		}
	}

	QDateTime EditDateDialog::GetDateTime () const
	{
		QDateTime result;
		result.setDate (Ui_.DateEdit_->selectedDate ());
		result.setTime (Ui_.TimeEdit_->time ());
		return result;
	}
}
}
