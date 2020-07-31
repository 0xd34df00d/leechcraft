/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QThread>
#include <QStringList>
#include <interfaces/core/icoreproxyfwd.h>

namespace LC
{
namespace NewLife
{
namespace Importers
{
	class KopeteImportThread : public QThread
	{
		Q_OBJECT

		const ICoreProxy_ptr Proxy_;
		QString Proto_;
		QStringList Files_;
	public:
		KopeteImportThread (const ICoreProxy_ptr& proxy, const QString& proto, const QStringList& files);
	protected:
		void run ();
	private:
		void ParseFile (const QString&);
	};
}
}
}
