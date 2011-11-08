/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#include <util/tagscompleter.h>
#include <util/tagscompletionmodel.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/itagsmanager.h>
#include "core.h"
#include "addfeed.h"

namespace LeechCraft
{
namespace Aggregator
{
	using LeechCraft::Util::TagsCompleter;

	AddFeed::AddFeed (const QString& url, QWidget *parent)
	: QDialog (parent)
	{
		setupUi (this);
		new TagsCompleter (Tags_, this);
		Tags_->AddSelector ();

		URL_->setText (url);
	}

	QString AddFeed::GetURL () const
	{
		QString result = URL_->text ().simplified ();
		if (result.startsWith ("itpc"))
			result.replace (0, 4, "http");
		return result;			
	}

	QStringList AddFeed::GetTags () const
	{
		return Core::Instance ().GetProxy ()->GetTagsManager ()->Split (Tags_->text ());
	}
}
}
