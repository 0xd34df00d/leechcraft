/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <util/threads/coro/taskfwd.h>
#include "interfaces/monocle/idocument.h"
#include "defaultbackendmanager.h"

namespace LC::Monocle
{
	class DocumentLoader : public QObject
	{
		DefaultBackendManager BackendManager_;
		QObjectList Backends_;
	public:
		using QObject::QObject;

		void AddPlugin (QObject*);

		DefaultBackendManager& GetDefaultBackendManager ();

		bool CanHandleMime (const QString&) const;
		bool CanLoadDocument (const QString&) const;
		Util::ContextTask<IDocument_ptr> LoadDocument (QString);
	};
}
