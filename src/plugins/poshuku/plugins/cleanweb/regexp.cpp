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

#include "regexp.h"
#include <QtDebug>

namespace LeechCraft
{
namespace Poshuku
{
namespace CleanWeb
{
#ifdef USE_PCRE
	namespace
	{
		pcre* Compile (const QString& str, Qt::CaseSensitivity cs)
		{
			const char *error = 0;
			int errOffset = 0;
			int options = PCRE_UTF8;
			if (cs == Qt::CaseInsensitive)
				options |= PCRE_CASELESS;
			auto re = pcre_compile (str.toUtf8 ().constData (), options, &error, &errOffset, NULL);
			if (!re)
				qWarning () << Q_FUNC_INFO
						<< "failed compiling"
						<< str
						<< error;
			return re;
		}
	}
#endif

	bool RegExp::IsFast ()
	{
#ifdef USE_PCRE
		return true;
#else
		return false;
#endif
	}

	RegExp::RegExp ()
#ifdef USE_PCRE
	: PRx_ (0)
#endif
	{
	}

	RegExp::RegExp (const RegExp& rx)
#ifdef USE_PCRE
	: Pattern_ (rx.Pattern_)
	, CS_ (rx.CS_)
	, PRx_ (Compile (Pattern_, CS_))
#else
	: Rx_ (rx)
#endif
	{
	}

	RegExp::RegExp (const QString& str, Qt::CaseSensitivity cs)
#ifdef USE_PCRE
	: Pattern_ (str)
	, CS_ (cs)
	, PRx_ (Compile (Pattern_, CS_))
#else
	: Rx_ (str, cs, QRegExp::RegExp)
#endif
	{
	}

	RegExp::~RegExp ()
	{
#ifdef USE_PCRE
		if (PRx_)
			pcre_free (PRx_);
#endif
	}

	RegExp& RegExp::operator= (const RegExp& rx)
	{
#ifdef USE_PCRE
		Pattern_ = rx.Pattern_;
		CS_ = rx.CS_;
		if (PRx_)
			pcre_free (PRx_);
		PRx_ = Compile (Pattern_, CS_);
#else
		Rx_ = rx.Rx_;
#endif
		return *this;
	}

	bool RegExp::Matches (const QString& str) const
	{
#ifdef USE_PCRE
		const auto& utf8 = str.toUtf8 ();
		return pcre_exec (PRx_, NULL, utf8.constData (), utf8.size (), 0, 0, NULL, 0) >= 0;
#else
		return Rx_.exactMatch (str);
#endif
	}

	QString RegExp::GetPattern () const
	{
#ifdef USE_PCRE
		return Pattern_;
#else
		return Rx_.pattern ();
#endif
	}

	Qt::CaseSensitivity RegExp::GetCaseSensitivity () const
	{
#ifdef USE_PCRE
		return CS_;
#else
		return Rx_.caseSensitivity ();
#endif
	}
}
}
}