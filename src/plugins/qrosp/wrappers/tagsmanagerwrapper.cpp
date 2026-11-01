/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "tagsmanagerwrapper.h"
#include <interfaces/core/itagsmanager.h>
#include <util/tags/emitters/tagsmanager.h>

namespace LC
{
namespace Qrosp
{
	TagsManagerWrapper::TagsManagerWrapper (ITagsManager *manager)
	: Manager_ (manager)
	{
		connect (&Manager_->GetEmitter (),
				&Util::Emitters::TagsManager::nameReplaced,
				this,
				&TagsManagerWrapper::nameReplaced);
	}

	QString TagsManagerWrapper::GetID (const QString& tag)
	{
		return Manager_->GetID (tag).transform ([] (const TagId& id) { return id.Id_; }).value_or (QString {});
	}

	QStringList TagsManagerWrapper::GetTags (const QString& id) const
	{
		return Manager_->GetTags (TagId { id });
	}

	QStringList TagsManagerWrapper::GetAllTags () const
	{
		return Manager_->GetAllTags ();
	}

	QStringList TagsManagerWrapper::Split (const QString& string) const
	{
		return Manager_->Split (string);
	}

	QString TagsManagerWrapper::Join (const QStringList& tags) const
	{
		return Manager_->Join (tags);
	}

	QAbstractItemModel* TagsManagerWrapper::GetModel ()
	{
		return Manager_->GetModel ();
	}
}
}
