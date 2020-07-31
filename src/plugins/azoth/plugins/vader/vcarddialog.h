/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "ui_vcarddialog.h"

namespace LC
{
namespace Azoth
{
namespace Vader
{
	class VCardDialog : public QDialog
	{
		Q_OBJECT

		Ui::VCardDialog Ui_;
	public:
		VCardDialog (QWidget* = 0);

		void SetAvatar (const QImage&);
		void SetInfo (QMap<QString, QString>);
	};
}
}
}
