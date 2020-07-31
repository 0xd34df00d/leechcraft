/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "imhistimporterbase.h"
#include <QTimer>
#include <interfaces/structures.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ientitymanager.h>

namespace LC
{
namespace NewLife
{
	IMHistImporterBase::IMHistImporterBase (const ICoreProxy_ptr& proxy, QObject *parent)
	: QObject (parent)
	, Proxy_ (proxy)
	{
		QTimer::singleShot (1000,
				this,
				SLOT (doChunk ()));
	}

	void IMHistImporterBase::doChunk ()
	{
		const Entity& e = GetEntityChunk ();
		if (e.Additional_.isEmpty ())
		{
			deleteLater ();
			return;
		}

		Proxy_->GetEntityManager ()->HandleEntity (e);
		QTimer::singleShot (500,
				this,
				SLOT (doChunk ()));
	}
}
}
