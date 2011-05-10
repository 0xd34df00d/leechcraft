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

#include "userscript.h"
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <QtDebug>
#include <QFile>
#include <QHash>
#include <QTextStream>
#include "greasemonkey.h"


namespace LeechCraft
{
namespace Poshuku
{
namespace FatApe
{
	const QString MetadataStart = "// ==UserScript==";
	const QString MetadataEnd = "// ==/UserScript==";

	template<typename InputIterator, typename Function>
	bool any (InputIterator first, InputIterator last, Function f)
	{
		for (; first != last; ++first)
			if (f (*first))
				return true;
		return false;
	}

	UserScript::UserScript (const QString& scriptPath)
	: ScriptPath_ (scriptPath)
	, MetadataRX_ ("//\\s+@(\\S*)\\s+(.*)", Qt::CaseInsensitive)
	{
		ParseMetadata ();
		if (!Metadata_.count ("include"))
			Metadata_.insert ("include", "*");
	}

	UserScript::UserScript (const UserScript& script)
	: MetadataRX_ ("//\\s+@(\\S*)\\s+(.*)", Qt::CaseInsensitive)
	{
		ScriptPath_ = script.ScriptPath_;
		Metadata_ = script.Metadata_;
	}

	void UserScript::ParseMetadata ()
	{
		QFile script (ScriptPath_);

		if (!script.open (QFile::ReadOnly))
		{
			qWarning () << Q_FUNC_INFO
				<< "unable to open file"
				<< script.fileName ()
				<< "for reading:"
				<< script.errorString ();
			return;
		}

		QTextStream content (&script);
		QString line;

		if (content.readLine () != MetadataStart)
			return;

		while((line = content.readLine ()) != MetadataEnd && !content.atEnd ())
		{
			MetadataRX_.indexIn (line);
			QString key (MetadataRX_.cap (1).trimmed ());
			QString value (MetadataRX_.cap (2).trimmed ());

			Metadata_.insert (key, value);
		}			
	
	}

	void UserScript::BuildPatternsList (QList<QRegExp>& list, bool include) const
	{
		Q_FOREACH (const QString& pattern, Metadata_.values (include ? "include" : "exclude"))
			list.append (QRegExp (pattern, Qt::CaseInsensitive, QRegExp::Wildcard));
	}

	bool UserScript::MatchToPage (const QString& pageUrl) const
	{
		QList<QRegExp> include;
		QList<QRegExp> exclude;
		boost::function<bool (QRegExp&)> match = 
			boost::bind (&QRegExp::indexIn,
				_1,
				pageUrl,
				0,
				QRegExp::CaretAtZero
			) != -1;
				
		BuildPatternsList (include);
		BuildPatternsList (exclude, false);
		
		return	any (include.begin (), include.end (), match) && 
				!any (exclude.begin (), exclude.end (), match);


	}

	void UserScript::Inject( QWebFrame* frame ) const
	{
		QFile script (ScriptPath_);

		if (script.open (QFile::ReadOnly))
		{
			QTextStream content (&script);
			QString gmLayerId = QString ("Greasemonkey%1%2")
					.arg (qHash (Namespace ()))
					.arg (qHash (Name ()));
			QString toInject = QString ("(function (){"
				"var GM_addStyle = %1.addStyle;"
				"%2})()")
					.arg (gmLayerId)
					.arg (content.readAll ());


			frame->addToJavaScriptWindowObject (gmLayerId, 
					new GreaseMonkey (frame, Namespace (), Name ()));
			frame->evaluateJavaScript (toInject);
		}
	}

	const QString UserScript::Name () const
	{
		return Metadata_.value ("name", "");
	}

	const QString UserScript::Description () const
	{
		return Metadata_.value ("description", "");
	}

	const QString UserScript::Namespace () const
	{
		return Metadata_.value ("namespace", "Default namespace");
	}
}
}
}

