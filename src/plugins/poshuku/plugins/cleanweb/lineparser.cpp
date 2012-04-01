/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#include "lineparser.h"
#include <QtDebug>
#include "filter.h"

namespace LeechCraft
{
namespace Poshuku
{
namespace CleanWeb
{
	LineParser::LineParser (Filter *f)
	: Filter_ (f)
	, Total_ (0)
	, Success_ (0)
	{
	}

	int LineParser::GetTotal () const
	{
		return Total_;
	}

	int LineParser::GetSuccess () const
	{
		return Success_;
	}

	void LineParser::operator() (const QString& line)
	{
		if (line.startsWith ('!'))
			return;

		++Total_;

		QString actualLine;
		FilterOption f = FilterOption ();
		bool cs = false;
		if (line.indexOf ('$') != -1)
		{
			const QStringList& splitted = line.split ('$',
					QString::SkipEmptyParts);

			if (splitted.size () != 2)
			{
				qWarning () << Q_FUNC_INFO
					<< "incorrect usage of $-pattern:"
					<< splitted.size ()
					<< line;
				return;
			}

			actualLine = splitted.at (0);
			QStringList options = splitted.at (1).split (',',
					QString::SkipEmptyParts);

			if (options.removeAll ("match-case"))
			{
				f.Case_ = Qt::CaseSensitive;
				cs = true;
			}

			if (options.removeAll ("third-party"))
				f.AbortForeign_ = true;

			Q_FOREACH (const QString& option, options)
				if (option.startsWith ("domain="))
				{
					QString domain = option;
					domain.remove (0, 7);
					if (domain.startsWith ('~'))
						f.NotDomains_ << domain.remove (0, 1);
					else
						f.Domains_ << domain;
					options.removeAll (option);
				}

			if (options.size ())
			{
				/*
				qWarning () << Q_FUNC_INFO
						<< "unsupported options for filter"
						<< actualLine
						<< options;
						*/
				return;
			}
		}
		else
			actualLine = line;

		bool white = false;
		if (actualLine.startsWith ("@@"))
		{
			actualLine.remove (0, 2);
			white = true;
		}

		if (actualLine.startsWith ('/') &&
				actualLine.endsWith ('/'))
		{
			actualLine = actualLine.mid (1, actualLine.size () - 2);
			f.MatchType_ = FilterOption::MTRegexp;
		}
		else
		{
			if (actualLine.endsWith ('|'))
			{
				actualLine.chop (1);
				actualLine.prepend ('*');
			}
			else if (actualLine.startsWith ('|'))
			{
				actualLine.remove (0, 1);
				actualLine.append ('*');
			}
			else if (actualLine.contains ('*') ||
					actualLine.contains ('?'))
			{
				actualLine.prepend ('*');
				actualLine.append ('*');
			}
			else
				f.MatchType_ = FilterOption::MTPlain;
			actualLine.replace ('?', "\\?");
		}

		if (white)
			Filter_->ExceptionStrings_ << (cs ? actualLine : actualLine.toLower ());
		else
			Filter_->FilterStrings_ << (cs ? actualLine : actualLine.toLower ());

		if (FilterOption () != f)
			Filter_->Options_ [actualLine] = f;

		if (f.MatchType_ == FilterOption::MTRegexp)
			Filter_->RegExps_ [actualLine] = QRegExp (actualLine, f.Case_, QRegExp::RegExp);

		++Success_;
	}
}
}
}