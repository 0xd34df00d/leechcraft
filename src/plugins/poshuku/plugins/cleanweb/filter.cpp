/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "filter.h"
#include <QDataStream>
#include <QtDebug>
#include <util/sll/unreachable.h>

namespace LC
{
namespace Poshuku
{
namespace CleanWeb
{
	QDataStream& operator<< (QDataStream& out, const FilterOption& opt)
	{
		qint8 version = 3;
		out << version
			<< static_cast<qint8> (opt.Case_)
			<< static_cast<qint8> (opt.MatchType_)
			<< opt.Domains_
			<< opt.NotDomains_
			<< static_cast<qint8> (opt.ThirdParty_);
		return out;
	}

	QDataStream& operator>> (QDataStream& in, FilterOption& opt)
	{
		qint8 version = 0;
		in >> version;

		if (version < 1 || version > 3)
		{
			qWarning () << Q_FUNC_INFO
				<< "unknown version"
				<< version;
			return in;
		}

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

		if (version == 2)
		{
			bool abort = false;
			in >> abort;
			opt.ThirdParty_ = abort ?
					FilterOption::ThirdParty::Yes :
					FilterOption::ThirdParty::Unspecified;
		}
		if (version >= 3)
		{
			qint8 tpVal;
			in >> tpVal;
			opt.ThirdParty_ = static_cast<FilterOption::ThirdParty> (tpVal);
		}

		return in;
	}

	bool operator== (const FilterOption& f1, const FilterOption& f2)
	{
		return f1.ThirdParty_ == f2.ThirdParty_ &&
				f1.Case_ == f2.Case_ &&
				f1.MatchType_ == f2.MatchType_ &&
				f1.Domains_ == f2.Domains_ &&
				f1.NotDomains_ == f2.NotDomains_;
	}

	bool operator!= (const FilterOption& f1, const FilterOption& f2)
	{
		return !(f1 == f2);
	}

	namespace
	{
		const char* HumanReadable (FilterOption::ThirdParty opt)
		{
			switch (opt)
			{
			case FilterOption::ThirdParty::Yes:
				return "yes";
			case FilterOption::ThirdParty::No:
				return "no";
			case FilterOption::ThirdParty::Unspecified:
				return "unspecified";
			}

			Util::Unreachable ();
		}
	}

	QDebug operator<< (QDebug dbg, const FilterOption& option)
	{
		QDebugStateSaver saver { dbg };
		dbg.nospace () << "FilterOption { "
				<< "CS: " << (option.Case_ == Qt::CaseSensitive) << "; "
				<< "match type: " << static_cast<int> (option.MatchType_) << "; "
				<< "match objects: " << option.MatchObjects_ << "; "
				<< "domains: " << option.Domains_ << "; "
				<< "!domains: " << option.NotDomains_ << "; "
				<< "selector: " << option.HideSelector_ << "; "
				<< "third party requests: " << HumanReadable (option.ThirdParty_)
				<< " }";
		return dbg;
	}

	QDebug operator<< (QDebug dbg, const FilterItem& item)
	{
		dbg << "FilterItem {"
				<< "RX:" << item.RegExp_.GetPattern () << "; "
				<< "Plain: " << item.PlainMatcher_ << ": "
				<< "Opts: " << item.Option_
				<< "}";
		return dbg;
	}

	QDataStream& operator<< (QDataStream& out, const FilterItem& item)
	{
		out << static_cast<quint8> (2)
			<< QString::fromUtf8 (item.PlainMatcher_)
			<< item.RegExp_.GetPattern ()
			<< static_cast<quint8> (item.RegExp_.GetCaseSensitivity ())
			<< item.Option_;
		return out;
	}

	QDataStream& operator>> (QDataStream& in, FilterItem& item)
	{
		quint8 version = 0;
		in >> version;
		if (version != 2)
		{
			qWarning () << "unknown version" << version;
			return in;
		}

		QString origStr;
		in >> origStr;
		item.PlainMatcher_ = origStr.toUtf8 ();
		if (version == 2)
		{
			QString str;
			quint8 cs;
			in >> str >> cs;
			item.RegExp_ = Util::RegExp (str, static_cast<Qt::CaseSensitivity> (cs));
		}
		in >> item.Option_;
		return in;
	}

	Filter& Filter::operator+= (const Filter& f)
	{
		Filters_ << f.Filters_;
		Exceptions_ << f.Exceptions_;
		return *this;
	}
}
}
}
