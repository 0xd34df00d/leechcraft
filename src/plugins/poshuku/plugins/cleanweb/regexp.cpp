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
#include "xmlsettingsmanager.h"
#include <QtDebug>

#ifdef USE_PCRE
#include <pcre.h>
#endif

namespace LeechCraft
{
namespace Poshuku
{
namespace CleanWeb
{
#ifdef USE_PCRE

#ifndef PCRE_STUDY_JIT_COMPILE
#define PCRE_STUDY_JIT_COMPILE 0
#endif

	class PCREWrapper
	{
		pcre *RE_;
		pcre_extra *Extra_;
	public:
		PCREWrapper ()
		: RE_ (0)
		, Extra_ (0)
		{
		}

		PCREWrapper (const QString& str, Qt::CaseSensitivity cs)
		: RE_ (Compile (str, cs))
		, Extra_ (0)
		{
			if (RE_)
			{
				pcre_refcount (RE_, 1);
				const char *error = 0;
				const int opts = XmlSettingsManager::Instance ()->property ("EnableJIT").toBool () ?
						PCRE_STUDY_JIT_COMPILE :
						0;
				Extra_ = pcre_study (RE_, opts, &error);
			}
		}

		PCREWrapper (const PCREWrapper& other)
		: RE_ (0)
		, Extra_ (0)
		{
			*this = other;
		}

		PCREWrapper& operator= (const PCREWrapper& other)
		{
			if (RE_ && !pcre_refcount (RE_, -1))
			{
				FreeStudy ();
				pcre_free (RE_);
			}

			RE_ = other.RE_;
			Extra_ = other.Extra_;
			if (RE_)
				pcre_refcount (RE_, 1);

			return *this;
		}

		~PCREWrapper ()
		{
			if (!RE_)
				return;

			if (!pcre_refcount (RE_, -1))
			{
				FreeStudy ();
				pcre_free (RE_);
			}
		}

		int Exec (const QByteArray& utf8) const
		{
			return RE_ ? pcre_exec (RE_, Extra_, utf8.constData (), utf8.size (), 0, 0, NULL, 0) : -1;
		}
	private:
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

		void FreeStudy ()
		{
			if (Extra_)
#ifdef PCRE_CONFIG_JIT
				pcre_free_study (Extra_);
#else
				pcre_free (Extra_);
#endif
		}
	};
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
	{
	}

	RegExp::RegExp (const RegExp& rx)
#ifdef USE_PCRE
	: Pattern_ (rx.Pattern_)
	, CS_ (rx.CS_)
	, PRx_ (rx.PRx_)
#else
	: Rx_ (rx.Rx_)
#endif
	{
	}

	RegExp::RegExp (const QString& str, Qt::CaseSensitivity cs)
#ifdef USE_PCRE
	: Pattern_ (str)
	, CS_ (cs)
	, PRx_ (new PCREWrapper (str, cs))
#else
	: Rx_ (str, cs, QRegExp::RegExp)
#endif
	{
	}

	RegExp::~RegExp ()
	{
	}

	RegExp& RegExp::operator= (const RegExp& rx)
	{
#ifdef USE_PCRE
		Pattern_ = rx.Pattern_;
		CS_ = rx.CS_;
		PRx_ = rx.PRx_;
#else
		Rx_ = rx.Rx_;
#endif
		return *this;
	}

	bool RegExp::Matches (const QString& str) const
	{
#ifdef USE_PCRE
		return PRx_->Exec (str.toUtf8 ()) >= 0;
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
