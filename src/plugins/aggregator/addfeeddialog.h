/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "ui_addfeeddialog.h"

namespace LC
{
namespace Aggregator
{
	class AddFeedDialog : public QDialog
	{
		Ui::AddFeedDialog Ui_;
	public:
		explicit AddFeedDialog (const QString& = {}, QWidget *parent = nullptr);

		QString GetURL () const;
		QStringList GetTags () const;
	};
}
}
