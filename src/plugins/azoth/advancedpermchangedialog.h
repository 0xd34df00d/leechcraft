/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "ui_advancedpermchangedialog.h"

namespace LC
{
namespace Azoth
{
	class ICLEntry;

	class AdvancedPermChangeDialog : public QDialog
	{
		Ui::AdvancedPermChangeDialog Ui_;
	public:
		AdvancedPermChangeDialog (const QList<ICLEntry*>& entry,
				const QByteArray& permClass, const QByteArray& perm, QWidget* = 0);

		QString GetReason () const;
		bool IsGlobal () const;
	};
}
}
