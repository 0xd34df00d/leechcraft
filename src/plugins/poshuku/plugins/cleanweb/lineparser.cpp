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

	namespace
	{
		QStringList ParseOptions (QStringList options, FilterOption& f)
		{
			if (options.removeAll ("match-case"))
				f.Case_ = Qt::CaseSensitive;

			if (options.removeAll ("third-party"))
				f.AbortForeign_ = true;

			if (options.removeAll ("~third-party"))
				f.AbortForeign_ = false;

			Q_FOREACH (const QString& option, options)
				if (option.startsWith ("domain="))
				{
					const auto& domain = option.mid (7);
					if (domain.startsWith ('~'))
						f.NotDomains_ << domain.mid (1);
					else
						f.Domains_ << domain;
					options.removeAll (option);
				}

			auto handleSubobj = [&options, &f] (const QString& name, FilterOption::MatchObject obj)
			{
				if (options.removeAll (name))
					f.MatchObjects_ |= obj;
				if (options.removeAll ("~" + name))
					f.MatchObjects_ |= ~FilterOption::MatchObjects (obj);
			};
			handleSubobj ("image", FilterOption::MatchObject::Image);
			handleSubobj ("script", FilterOption::MatchObject::Script);
			handleSubobj ("object", FilterOption::MatchObject::Object);
			handleSubobj ("stylesheet", FilterOption::MatchObject::CSS);
			handleSubobj ("object-subrequest", FilterOption::MatchObject::ObjSubrequest);
			handleSubobj ("subdocument", FilterOption::MatchObject::Subdocument);
			handleSubobj ("xmlhttprequest", FilterOption::MatchObject::AJAX);
			handleSubobj ("popup", FilterOption::MatchObject::Popup);

			return options;
		}

		void ParseWithOption (QString actualLine, FilterOption f, QList<FilterItem>& items)
		{
			if (actualLine.startsWith ('/') &&
					actualLine.endsWith ('/'))
			{
				actualLine = actualLine.mid (1, actualLine.size () - 2);
				f.MatchType_ = FilterOption::MTRegexp;
				const FilterItem item
				{
					actualLine.toUtf8 (),
					RegExp (actualLine, f.Case_),
					QByteArrayMatcher (),
					f
				};
				items << item;
				return;
			}

			while (!actualLine.isEmpty () && actualLine.at (0) == '*')
				actualLine = actualLine.mid (1);
			while (!actualLine.isEmpty () && actualLine.at (actualLine.size () - 1) == '*')
				actualLine.chop (1);

			if (actualLine.startsWith ("||"))
			{
				auto spawned = "." + actualLine.mid (2);
				if (f.Case_ == Qt::CaseInsensitive)
					spawned = spawned.toLower ();
				f.MatchType_ = FilterOption::MTPlain;
				ParseWithOption (spawned, f, items);

				actualLine = actualLine.mid (2);
				actualLine.prepend ('/');
			}

			if (actualLine.endsWith ('|') && actualLine.startsWith ('|'))
				actualLine = actualLine.mid (1, actualLine.size () - 2);
			else if (actualLine.endsWith ('|'))
			{
				actualLine.chop (1);
				if (actualLine.contains ('*'))
				{
					actualLine.prepend ('*');
					f.MatchType_ = FilterOption::MTWildcard;
				}
				else
					f.MatchType_ = FilterOption::MTEnd;
			}
			else if (actualLine.startsWith ('|'))
			{
				actualLine.remove (0, 1);
				if (actualLine.contains ('*'))
				{
					actualLine.append ('*');
					f.MatchType_ = FilterOption::MTWildcard;
				}
				else
					f.MatchType_ = FilterOption::MTBegin;
			}
			else
			{
				if (actualLine.contains ('*'))
				{
					actualLine.prepend ('*');
					actualLine.append ('*');
					f.MatchType_ = FilterOption::MTWildcard;
				}
				else
					f.MatchType_ = FilterOption::MTPlain;
			}

			if (f.MatchType_ == FilterOption::MTWildcard)
				actualLine.replace ('?', "\\?");

			if (f.MatchType_ != FilterOption::MTRegexp && actualLine.contains ('^'))
			{
				if (!RegExp::IsFast ())
					return;

				actualLine.replace ('*', ".*");
				if (f.MatchType_ != FilterOption::MTWildcard)
					actualLine.replace ('?', "\\?");
				switch (f.MatchType_)
				{
				case FilterOption::MTEnd:
					actualLine.prepend (".*");
					break;
				case FilterOption::MTBegin:
					actualLine.append (".*");
					break;
				case FilterOption::MTPlain:
					actualLine.prepend (".*");
					actualLine.append (".*");
					break;
				case FilterOption::MTWildcard:
				case FilterOption::MTRegexp:
					break;
				}
				actualLine.replace ('^', "[^a-zA-Z0-9_\\.%-]");
				f.MatchType_ = FilterOption::MTRegexp;
			}

			const auto& itemRx = f.MatchType_ == FilterOption::MTRegexp ?
					RegExp (actualLine, f.Case_) :
					RegExp ();
			const QByteArrayMatcher matcher = f.MatchType_ == FilterOption::MTPlain ?
					QByteArrayMatcher (actualLine.toUtf8 ()) :
					QByteArrayMatcher ();
			const FilterItem item
			{
				(f.Case_ == Qt::CaseSensitive ? actualLine : actualLine.toLower ()).toUtf8 (),
				itemRx,
				matcher,
				f
			};

			items << item;
		}
	}

	void LineParser::operator() (const QString& line)
	{
		if (line.startsWith ('!'))
			return;

		++Total_;

		QString actualLine = line;
		QStringList additionalLines;
		FilterOption f = FilterOption ();

		if (actualLine.contains ("##"))
		{
			const auto& split = actualLine.split ("##");
			if (split.size () != 2)
			{
				qWarning () << Q_FUNC_INFO
					<< "incorrect usage of ##-pattern:"
					<< split.size ()
					<< line;
				return;
			}

			actualLine = split.at (0);
			f.HideSelector_ = split.at (1);
		}

		if (actualLine.contains ('$'))
		{
			const auto& splitted = actualLine.split ('$', QString::SkipEmptyParts);

			if (splitted.size () != 2)
			{
				qWarning () << Q_FUNC_INFO
					<< "incorrect usage of $-pattern:"
					<< splitted.size ()
					<< actualLine;
				return;
			}

			actualLine = splitted.at (0);

			const auto& remaining = ParseOptions (splitted.at (1).split (',', QString::SkipEmptyParts), f);
			if (remaining.size ())
			{
				qWarning () << Q_FUNC_INFO
						<< "unsupported options for filter"
						<< actualLine
						<< remaining;
				return;
			}
		}

		bool white = false;
		if (actualLine.startsWith ("@@"))
		{
			actualLine.remove (0, 2);
			white = true;
		}

		ParseWithOption (actualLine, f, white ? Filter_->Exceptions_ : Filter_->Filters_);

		++Success_;
	}
}
}
}
