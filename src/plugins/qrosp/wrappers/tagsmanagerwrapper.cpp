/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "tagsmanagerwrapper.h"
#include <interfaces/core/itagsmanager.h>

namespace LC
{
namespace Qrosp
{
	TagsManagerWrapper::TagsManagerWrapper (ITagsManager *manager)
	: Manager_ (manager)
	{
		connect (Manager_->GetQObject (),
				SIGNAL (tagsUpdated (const QStringList&)),
				this,
				SIGNAL (tagsUpdated (const QStringList&)));
	}

	QString TagsManagerWrapper::GetID (const QString& tag)
	{
		return Manager_->GetID (tag);
	}

	QString TagsManagerWrapper::GetTag (const QString& id) const
	{
		return Manager_->GetTag (id);
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

	QObject* TagsManagerWrapper::GetQObject ()
	{
		return Manager_->GetQObject ();
	}
}
}
