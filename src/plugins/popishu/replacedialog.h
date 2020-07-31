/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "ui_replacedialog.h"

namespace LC
{
namespace Popishu
{
	class ReplaceDialog : public QDialog
	{
		Q_OBJECT

		Ui::ReplaceDialog Ui_;
	public:
		enum class Scope
		{
			All,
			Selected
		};

		ReplaceDialog (QWidget* = 0);

		QString GetBefore () const;
		QString GetAfter () const;
		Qt::CaseSensitivity GetCaseSensitivity () const;
		Scope GetScope () const;
	};
}
}
