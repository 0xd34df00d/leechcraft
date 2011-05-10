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
#include <QSettings>
#include <QWebElement>



namespace LeechCraft
{
namespace Poshuku
{
namespace FatApe
{

	void GreaseMonkey::addStyle (const QString& css)
	{
		QWebElement body = Frame_->findFirstElement ("body");

		body.appendInside (QString ("<style type=\"text/css\">%1</style>").arg (css));
	}

	GreaseMonkey::GreaseMonkey (QWebFrame* frame, 
		const QString& scriptNamespace, const QString& scriptName)
	: Frame_ (frame)
	, ScriptNamespace_ (scriptNamespace)
	, ScriptName_ (scriptName)
	{
	}

	void GreaseMonkey::deleteValue (const QString& name)
	{
		QSettings settings (QCoreApplication::organizationName (),
			QCoreApplication::applicationName () + "_Poshuku_FatApe");

		settings.beginGroup (ScriptNamespace_);
		settings.beginGroup (ScriptName_);
		settings.remove (name);
		settings.endGroup ();
		settings.endGroup ();

	}

	QVariant GreaseMonkey::getValue (const QString& name )
	{
		return getValue (name, 0);
	}

	QVariant GreaseMonkey::getValue (const QString& name, QVariant default)
	{
		QSettings settings (QCoreApplication::organizationName (),
			QCoreApplication::applicationName () + "_Poshuku_FatApe");
		QVariant value;

		settings.beginGroup (ScriptNamespace_);
		settings.beginGroup (ScriptName_);
		value = settings.value (name, default);
		settings.endGroup ();
		settings.endGroup ();
		return value;
	}

	QVariant GreaseMonkey::listValues ()
	{
		QSettings settings (QCoreApplication::organizationName (),
			QCoreApplication::applicationName () + "_Poshuku_FatApe");
		QStringList values;

		settings.beginGroup (ScriptNamespace_);
		settings.beginGroup (ScriptName_);
		values = settings.allKeys ();
		settings.endGroup ();
		settings.endGroup ();
		
		return QVariant(values);
	}

	void GreaseMonkey::setValue (const QString& name, QVariant value)
	{
		QSettings settings (QCoreApplication::organizationName (),
			QCoreApplication::applicationName () + "_Poshuku_FatApe");

		settings.beginGroup (ScriptNamespace_);
		settings.beginGroup (ScriptName_);
		settings.setValue (name, value);
		settings.endGroup ();
		settings.endGroup ();
	}
}
}
}

