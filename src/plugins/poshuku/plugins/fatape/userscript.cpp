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
#include <QFile>
#include <QTextStream>

namespace LeechCraft
{
namespace Poshuku
{
namespace FatApe
{
	const char* MetadataStart = "// ==UserScript==";
	const char* MetadataEnd = "// ==/UserScript==";

	template<typename InputIterator, typename Function>
	bool any(InputIterator first, InputIterator last, Function f)
	{
		for (; first != last; ++first)
		{
			if (f (*first))
				return true;
		}
		return false;

	}

	UserScript::UserScript (const QString& scriptPath)
	: ScriptPath_ (scriptPath)
	, MetadataRX_ ("//\\s+@(\\S*)\\s+(.*)", Qt::CaseInsensitive)
	{
		ParseMetadata ();
		if (Metadata_.count ("include") == 0)
		{
			Metadata_.insert ("include", "*");
		}
	}

	UserScript::UserScript( const UserScript& script )
	: MetadataRX_ ("//\\s+@(\\S*)\\s+(.*)", Qt::CaseInsensitive)
	{
		ScriptPath_ = script.ScriptPath_;
		Metadata_ = script.Metadata_;
	}

	void UserScript::ParseMetadata ()
	{
		QFile script (ScriptPath_);

		if (script.open (QFile::ReadOnly))
		{
			QTextStream content (&script);
			QString line;

			if (content.readLine () != MetadataStart)
				return;

			while((line = content.readLine ()) != MetadataEnd && !content.atEnd ())
			{
				MetadataRX_.indexIn (line);
				QString key (MetadataRX_.cap (1));
				QString value (MetadataRX_.cap (2));

				Metadata_.insert (key, value);
			}			

		}	
	}

	void UserScript::BuildPatternsList( QList<QRegExp>& list, bool include /*= true*/ ) const
	{
		Q_FOREACH(const QString& pattern, Metadata_.values (include ? "include" : "exclude"))
		{
			list.append (QRegExp (pattern, Qt::CaseInsensitive, QRegExp::Wildcard));
		}

	}

	bool UserScript::MatchToPage (const QString& pageUrl) const
	{
		QList<QRegExp> include;
		QList<QRegExp> exclude;
		boost::function<bool (QRegExp&)> match = boost::bind (
				&QRegExp::indexIn,
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
			QString toInject = QString("(function(){%1})()").arg (content.readAll ());

			frame->evaluateJavaScript (toInject);
		}
	}

}
}
}

