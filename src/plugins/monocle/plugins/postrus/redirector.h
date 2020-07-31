/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/monocle/iredirectproxy.h>

class QProcess;

namespace LC
{
namespace Monocle
{
namespace Postrus
{
	class Redirector : public QObject
					 , public IRedirectProxy
	{
		Q_OBJECT

		const QString Source_;
		QProcess * const Process_;

		QString Target_;
	public:
		Redirector (const QString&);

		QObject* GetQObject ();
		QString GetRedirectSource () const;
		QString GetRedirectTarget () const;
		QString GetRedirectedMime () const;
	private:
		void StartConverting ();
		void HandleFinished ();
	signals:
		void ready (const QString& target);
	};
}
}
}
