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

#include "filter.h"
#include <QtDebug>

namespace LeechCraft
{
namespace Poshuku
{
namespace CleanWeb
{
	QDataStream& operator<< (QDataStream& out, const FilterOption& opt)
	{
		qint8 version = 2;
		out << version
			<< static_cast<qint8> (opt.Case_)
			<< static_cast<qint8> (opt.MatchType_)
			<< opt.Domains_
			<< opt.NotDomains_
			<< opt.AbortForeign_;
		return out;
	}

	QDataStream& operator>> (QDataStream& in, FilterOption& opt)
	{
		qint8 version = 0;
		in >> version;

		if (version < 1 || version > 2)
		{
			qWarning () << Q_FUNC_INFO
				<< "unknown version"
				<< version;
			return in;
		}

		if (version >= 1)
		{
			qint8 cs;
			in >> cs;
			opt.Case_ = cs ?
				Qt::CaseInsensitive :
				Qt::CaseSensitive;
			qint8 mt;
			in >> mt;
			opt.MatchType_ = static_cast<FilterOption::MatchType> (mt);
			in >> opt.Domains_
				>> opt.NotDomains_;
		}
		if (version >= 2)
			in >> opt.AbortForeign_;

		return in;
	}

	FilterOption::FilterOption ()
	: Case_ (Qt::CaseInsensitive)
	, MatchType_ (MTWildcard)
	, AbortForeign_ (false)
	{
	}

	bool operator== (const FilterOption& f1, const FilterOption& f2)
	{
		return f1.AbortForeign_ == f2.AbortForeign_ &&
			f1.Case_ == f2.Case_ &&
			f1.MatchType_ == f2.MatchType_ &&
			f1.Domains_ == f2.Domains_ &&
			f1.NotDomains_ == f2.NotDomains_;
	}

	bool operator!= (const FilterOption& f1, const FilterOption& f2)
	{
		return !(f1 == f2);
	}

	Filter& Filter::operator+= (const Filter& f)
	{
		ExceptionStrings_ << f.ExceptionStrings_;
		ExceptionStrings_.removeDuplicates ();
		FilterStrings_ << f.FilterStrings_;
		FilterStrings_.removeDuplicates ();

		Options_.unite (f.Options_);
		RegExps_.unite (f.RegExps_);

		return *this;
	}
}
}
}
