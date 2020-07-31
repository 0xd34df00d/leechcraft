/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/core/icoreproxyfwd.h>

namespace LC
{
struct Entity;

namespace NewLife
{
	class IMHistImporterBase : public QObject
	{
		Q_OBJECT

		const ICoreProxy_ptr Proxy_;
	public:
		IMHistImporterBase (const ICoreProxy_ptr&, QObject* = nullptr);
	protected:
		virtual Entity GetEntityChunk () = 0;
	protected slots:
		virtual void doChunk ();
	};
}
}
