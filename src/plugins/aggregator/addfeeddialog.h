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

class ITagsManager;

namespace LC
{
namespace Aggregator
{
	class AddFeedDialog : public QDialog
	{
		Ui::AddFeedDialog Ui_;

		const ITagsManager * const TagsManager_;
	public:
		explicit AddFeedDialog (const ITagsManager*, const QString& = QString (), QWidget *parent = 0);

		QString GetURL () const;
		QStringList GetTags () const;
	};
}
}
