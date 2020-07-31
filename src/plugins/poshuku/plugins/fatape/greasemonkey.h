/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QObject>
#include <QStringList>
#include <QSettings>
#include <QVariant>
#include <interfaces/poshuku/iproxyobject.h>
#include "userscript.h"

namespace LC
{
namespace Poshuku
{
class IWebView;

namespace FatApe
{
	class GreaseMonkey : public QObject
	{
		Q_OBJECT

		IWebView *View_;
		IProxyObject *Proxy_;
		UserScript Script_;

		const QString InitJS_;
	public:
		GreaseMonkey (IWebView *view, IProxyObject *proxy, const QString& initJs, const UserScript& script);
	private:
		std::shared_ptr<QSettings> GetStorage () const;
	public slots:
		void init ();

		void addStyle (QString css);
		void deleteValue (const QString& name);
		QVariant getValue (const QString& name);
		QVariant getValue (const QString& name, QVariant defVal);
		QVariant listValues ();
		void setValue (const QString& name, QVariant value);
		void openInTab (const QString& url);
		QString getResourceText (const QString& resourceName);
		QString getResourceURL (const QString& resourceName);
	};
}
}
}
