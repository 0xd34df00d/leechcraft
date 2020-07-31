/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "greasemonkey.h"
#include <QCoreApplication>
#include <QFile>
#include <QHash>
#include <QTextStream>
#include <QtDebug>
#include <QUrl>
#include <interfaces/poshuku/iwebview.h>

namespace LC
{
namespace Poshuku
{
namespace FatApe
{
	GreaseMonkey::GreaseMonkey (IWebView *view, IProxyObject *proxy,
			const QString& initJs, const UserScript& script)
	: View_ (view)
	, Proxy_ (proxy)
	, Script_ (script)
	, InitJS_ (initJs)
	{
	}

	std::shared_ptr<QSettings> GreaseMonkey::GetStorage () const
	{
		const auto& orgName = QCoreApplication::organizationName ();
		const auto& sName = QCoreApplication::applicationName () + "_Poshuku_FatApe_Storage";
		std::shared_ptr<QSettings> settings
		{
			new QSettings { orgName, sName },
			[] (QSettings *settings)
			{
				settings->endGroup ();
				settings->endGroup ();
				delete settings;
			}
		};
		settings->beginGroup (QString::number (qHash (Script_.Namespace ())));
		settings->beginGroup (QString::number (qHash (Script_.Name ())));
		return settings;
	}

	void GreaseMonkey::init ()
	{
		View_->EvaluateJS (InitJS_);
	}

	void GreaseMonkey::addStyle (QString css)
	{
		QString js;
		js += "var el = document.createElement('style');";
		js += "el.type = 'text/css';";
		js += "el.innerHTML = '* { " + css.replace ('\'', "\\'") + " }'";
		js += "document.body.insertBefore(el, document.body.firstChild);";
		View_->EvaluateJS (js);
	}

	void GreaseMonkey::deleteValue (const QString& name)
	{
		qDebug () << Q_FUNC_INFO << name;
		GetStorage ()->remove (name);
	}

	QVariant GreaseMonkey::getValue (const QString& name)
	{
		qDebug () << Q_FUNC_INFO << name;
		return getValue (name, QVariant ());
	}

	QVariant GreaseMonkey::getValue (const QString& name, QVariant defVal)
	{
		qDebug () << Q_FUNC_INFO << name << "with" << defVal;
		return GetStorage ()->value (name, defVal);
	}

	QVariant GreaseMonkey::listValues ()
	{
		return GetStorage ()->allKeys ();
	}

	void GreaseMonkey::setValue (const QString& name, QVariant value)
	{
		qDebug () << Q_FUNC_INFO << name << "to" << value;
		GetStorage ()->setValue (name, value);
	}

	void GreaseMonkey::openInTab (const QString& url)
	{
		if (Proxy_)
			Proxy_->OpenInNewTab (url);
	}

	QString GreaseMonkey::getResourceText (const QString& resourceName)
	{
		QFile resource (Script_.GetResourcePath (resourceName));

		return resource.open (QFile::ReadOnly) ?
			QTextStream (&resource).readAll () :
			QString ();
	}

	QString GreaseMonkey::getResourceURL (const QString& resourceName)
	{
		QSettings settings (QCoreApplication::organizationName (),
			QCoreApplication::applicationName () + "_Poshuku_FatApe");
		QString mimeType = settings.value (QString ("resources/%1/%2/%3")
				.arg (qHash (Script_.Namespace ()))
				.arg (Script_.Name ())
				.arg (resourceName)).toString ();
		QFile resource (Script_.GetResourcePath (resourceName));

		return resource.open (QFile::ReadOnly) ?
			QString ("data:%1;base64,%2")
				.arg (mimeType)
				.arg (QString (resource.readAll ().toBase64 ())
						//The result is a base64 encoded URI, which is then also URI encoded,
						//as suggested by Wikipedia(http://en.wikipedia.org/wiki/Base64#URL_applications),
						//because of "+" and "/" characters in the base64 alphabet.
						//http://wiki.greasespot.net/GM_getResourceURL#Returns
						.replace ("+", "%2B")
						.replace ("/", "%2F")) :
			QString ();
	}
}
}
}
