/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "greasemonkey.h"
#include <QCoreApplication>
#include <QFile>
#include <QHash>
#include <QSettings>
#include <QTextStream>
#include <QWebElement>

namespace LeechCraft
{
namespace Poshuku
{
namespace FatApe
{
	GreaseMonkey::GreaseMonkey (QWebFrame *frame, IProxyObject* proxy, const UserScript& script)
	: Frame_ (frame)
	, Proxy_ (proxy)
	, Script_ (script)
	{
	}

	void GreaseMonkey::addStyle (const QString& css)
	{
		QWebElement body = Frame_->findFirstElement ("body");

		body.appendInside (QString ("<style type=\"text/css\">%1</style>").arg (css));
	}

	void GreaseMonkey::deleteValue (const QString& name)
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Poshuku_FatApe");

		settings.remove (QString("%1/%2/%3")
				.arg (qHash (Script_.Namespace ()))
				.arg (Script_.Name ())
				.arg(name));
		
	}

	QVariant GreaseMonkey::getValue (const QString& name)
	{
		return getValue (name, QVariant ());
	}

	QVariant GreaseMonkey::getValue (const QString& name, QVariant defVal)
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Poshuku_FatApe");


		return settings.value (QString("%1/%2/%3")
				.arg (qHash (Script_.Namespace ()))
				.arg (Script_.Name ())
				.arg (name), defVal);
		

	}

	QVariant GreaseMonkey::listValues ()
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Poshuku_FatApe");
		QStringList values;
		

		settings.beginGroup (QString::number (qHash (Script_.Namespace ())));
		settings.beginGroup (Script_.Name ());
		values = settings.allKeys ();
		settings.endGroup ();
		settings.endGroup ();
		
		return QVariant(values.filter (QRegExp ("^(?!resources/).*")));
	}

	void GreaseMonkey::setValue (const QString& name, QVariant value)
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Poshuku_FatApe");

		settings.setValue (QString("%1/%2/%3")
				.arg (qHash (Script_.Namespace()))
				.arg (Script_.Name ())
				.arg (name), value);

	}

	void GreaseMonkey::openInTab (const QString& url)
	{
		if (Proxy_)
			Proxy_->OpenInNewTab (url);
	}

	QString GreaseMonkey::getResourceText (const QString& resourceName)
	{
		QFile resource(Script_.GetResourcePath (resourceName));

		return resource.open (QFile::ReadOnly) ? 
			QTextStream (&resource).readAll () :
			QString ();
	}

	QString GreaseMonkey::getResourceURL (const QString& resourceName)
	{
		QSettings settings (QCoreApplication::organizationName (),
			QCoreApplication::applicationName () + "_Poshuku_FatApe");
		QString mimeType = settings.value (QString ("%1/%2/resources/%3")
				.arg (qHash (Script_.Namespace ()))
				.arg (Script_.Name ())
				.arg (resourceName)).toString ();
		QFile resource(Script_.GetResourcePath (resourceName));
		
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
			QString();
	}

}
}
}
