/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include <util/tags/tagscompleter.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/itagsmanager.h>
#include "addfeeddialog.h"

namespace LC
{
namespace Aggregator
{
	AddFeedDialog::AddFeedDialog (const QString& url, QWidget *parent)
	: QDialog { parent }
	{
		Ui_.setupUi (this);
		new Util::TagsCompleter (Ui_.Tags_);
		Ui_.Tags_->AddSelector ();

		Ui_.URL_->setText (url);
	}

	QString AddFeedDialog::GetURL () const
	{
		QString result = Ui_.URL_->text ().simplified ();
		if (result.startsWith ("itpc"))
			result.replace (0, 4, "http");
		return result;
	}

	QStringList AddFeedDialog::GetTags () const
	{
		return GetProxyHolder ()->GetTagsManager ()->Split (Ui_.Tags_->text ());
	}
}
}
