/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "lineparser.h"
#include <QtDebug>
#include "filter.h"

namespace LC
{
namespace Poshuku
{
namespace CleanWeb
{
	LineParser::LineParser (Filter *f)
	: Filter_ (f)
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
				f.ThirdParty_ = FilterOption::ThirdParty::Yes;
			else if (options.removeAll ("~third-party"))
				f.ThirdParty_ = FilterOption::ThirdParty::No;
			else
				f.ThirdParty_ = FilterOption::ThirdParty::Unspecified;

			for (const auto& option : QStringList { options })
				if (option.startsWith ("domain="))
				{
					const auto& domains = option.mid (7);
					for (const auto& domain : domains.split ("|", Qt::SkipEmptyParts))
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

		void ParseWithOption (QString actualLine, FilterOption f, QList<FilterItem_ptr>& items)
		{
			if (actualLine.startsWith ('/') &&
					actualLine.endsWith ('/'))
			{
				actualLine = actualLine.mid (1, actualLine.size () - 2);
				f.MatchType_ = FilterOption::MatchType::Regexp;
				items << std::make_shared<FilterItem> (FilterItem { Util::RegExp (actualLine, f.Case_), {}, f });
				return;
			}

			while (!actualLine.isEmpty () && actualLine.at (0) == '*')
				actualLine = actualLine.mid (1);
			while (!actualLine.isEmpty () && actualLine.at (actualLine.size () - 1) == '*')
				actualLine.chop (1);

			if (actualLine.startsWith ("||"))
			{
				auto spawned = "." + actualLine.mid (2);
				if (spawned.contains ("||"))
				{
					qWarning () << Q_FUNC_INFO
							<< "buggy double-pipe filter"
							<< actualLine;
					return;
				}
				if (f.Case_ == Qt::CaseInsensitive)
					spawned = spawned.toLower ();
				f.MatchType_ = FilterOption::MatchType::Plain;
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
					f.MatchType_ = FilterOption::MatchType::Wildcard;
				}
				else
					f.MatchType_ = FilterOption::MatchType::End;
			}
			else if (actualLine.startsWith ('|'))
			{
				actualLine.remove (0, 1);
				if (actualLine.contains ('*'))
				{
					actualLine.append ('*');
					f.MatchType_ = FilterOption::MatchType::Wildcard;
				}
				else
					f.MatchType_ = FilterOption::MatchType::Begin;
			}
			else
			{
				if (actualLine.contains ('*'))
				{
					actualLine.prepend ('*');
					actualLine.append ('*');
					f.MatchType_ = FilterOption::MatchType::Wildcard;
				}
				else
					f.MatchType_ = FilterOption::MatchType::Plain;
			}

			if (f.MatchType_ == FilterOption::MatchType::Wildcard)
				actualLine.replace ('?', "\\?");

			if (f.MatchType_ != FilterOption::MatchType::Regexp && actualLine.contains ('^'))
			{
				if (!Util::RegExp::IsFast ())
					return;

				actualLine.replace (".", "\\.");
				actualLine.replace ('*', ".*");
				if (f.MatchType_ != FilterOption::MatchType::Wildcard)
					actualLine.replace ('?', "\\?");
				switch (f.MatchType_)
				{
				case FilterOption::MatchType::End:
					actualLine.prepend (".*");
					break;
				case FilterOption::MatchType::Begin:
					actualLine.append (".*");
					break;
				case FilterOption::MatchType::Plain:
					actualLine.prepend (".*");
					actualLine.append (".*");
					break;
				case FilterOption::MatchType::Wildcard:
				case FilterOption::MatchType::Regexp:
					break;
				}
				actualLine.replace ('^', "[/?=&:]");
				f.MatchType_ = FilterOption::MatchType::Regexp;
			}

			const auto& casedOrigStr = (f.Case_ == Qt::CaseSensitive ?
					actualLine :
					actualLine.toLower ()).toUtf8 ();
			const auto& itemRx = f.MatchType_ == FilterOption::MatchType::Regexp ?
					Util::RegExp (actualLine, f.Case_) :
					Util::RegExp ();
			items << std::make_shared<FilterItem> (FilterItem { itemRx, casedOrigStr, f });
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
			const auto& splitted = actualLine.split ('$', Qt::SkipEmptyParts);

			if (splitted.size () != 2)
			{
				qWarning () << Q_FUNC_INFO
					<< "incorrect usage of $-pattern:"
					<< splitted.size ()
					<< actualLine;
				return;
			}

			actualLine = splitted.at (0);

			const auto& remaining = ParseOptions (splitted.at (1).split (',', Qt::SkipEmptyParts), f);
			if (remaining.size ())
			{
				/*
				qWarning () << Q_FUNC_INFO
						<< "unsupported options for filter"
						<< actualLine
						<< remaining;
						*/
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
