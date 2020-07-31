/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <interfaces/core/icoreproxy.h>

namespace LC
{
namespace XProxy
{
	class UrlListScript;

	class ScriptsManager : public QObject
	{
		Q_OBJECT

		const ICoreProxy_ptr Proxy_;

		QList<UrlListScript*> Scripts_;
	public:
		ScriptsManager (const ICoreProxy_ptr&);

		QList<UrlListScript*> GetScripts () const;
		UrlListScript* GetScript (const QByteArray&) const;
	};
}
}
