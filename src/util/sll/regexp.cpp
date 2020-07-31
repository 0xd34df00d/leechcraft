/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "regexp.h"
#include <QDataStream>
#include <QtDebug>

#ifdef USE_PCRE
#include <pcre.h>
#else
#include <QRegExp>
#endif

namespace LC
{
namespace Util
{
#ifdef USE_PCRE

#ifndef PCRE_STUDY_JIT_COMPILE
#define PCRE_STUDY_JIT_COMPILE 0
#endif

	class PCREWrapper
	{
		pcre *RE_ = nullptr;
		pcre_extra *Extra_ = nullptr;

		QString Pattern_;
		Qt::CaseSensitivity CS_ = Qt::CaseSensitive;
	public:
		PCREWrapper () = default;

		PCREWrapper (const QString& str, Qt::CaseSensitivity cs)
		: RE_ (Compile (str, cs))
		, Pattern_ (str)
		, CS_ (cs)
		{
			if (RE_)
			{
				pcre_refcount (RE_, 1);
				const char *error = 0;
				const int opts = PCRE_STUDY_JIT_COMPILE;
				Extra_ = pcre_study (RE_, opts, &error);
			}
		}

		PCREWrapper (const PCREWrapper& other)
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

		const QString& GetPattern () const
		{
			return Pattern_;
		}

		Qt::CaseSensitivity GetCS () const
		{
			return CS_;
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

	namespace
	{
		struct RegExpRegisterGuard
		{
			RegExpRegisterGuard ()
			{
				qRegisterMetaType<RegExp> ("Util::RegExp");
				qRegisterMetaTypeStreamOperators<RegExp> ();
			}
		} Guard;
	}

	struct RegExpImpl
	{
#if USE_PCRE
		PCREWrapper PRx_;
#else
		QRegExp Rx_;
#endif
	};

	bool RegExp::IsFast ()
	{
#ifdef USE_PCRE
		return true;
#else
		return false;
#endif
	}

	RegExp::RegExp (const QString& str, Qt::CaseSensitivity cs)
#ifdef USE_PCRE
	: Impl_ { new RegExpImpl { { str, cs } } }
#else
	: Impl_ { new RegExpImpl { QRegExp { str, cs, QRegExp::RegExp } } }
#endif
	{
	}

	bool RegExp::Matches (const QString& str) const
	{
		if (!Impl_)
			return {};

#ifdef USE_PCRE
		return Impl_->PRx_.Exec (str.toUtf8 ()) >= 0;
#else
		return Impl_->Rx_.exactMatch (str);
#endif
	}

	bool RegExp::Matches (const QByteArray& ba) const
	{
		if (!Impl_)
			return {};

#ifdef USE_PCRE
		return Impl_->PRx_.Exec (ba) >= 0;
#else
		return Impl_->Rx_.exactMatch (ba);
#endif
	}

	QString RegExp::GetPattern () const
	{
		if (!Impl_)
			return {};

#ifdef USE_PCRE
		return Impl_->PRx_.GetPattern ();
#else
		return Impl_->Rx_.pattern ();
#endif
	}

	Qt::CaseSensitivity RegExp::GetCaseSensitivity () const
	{
		if (!Impl_)
			return {};

#ifdef USE_PCRE
		return Impl_->PRx_.GetCS ();
#else
		return Impl_->Rx_.caseSensitivity ();
#endif
	}
}
}

QDataStream& operator<< (QDataStream& out, const LC::Util::RegExp& rx)
{
	out << static_cast<quint8> (1);
	out << rx.GetPattern ()
		<< static_cast<quint8> (rx.GetCaseSensitivity ());
	return out;
}

QDataStream& operator>> (QDataStream& in, LC::Util::RegExp& rx)
{
	quint8 version = 0;
	in >> version;
	if (version != 1)
	{
		qWarning () << Q_FUNC_INFO
				<< "unknown version"
				<< version;
		return in;
	}

	QString pattern;
	quint8 cs;
	in >> pattern
		>> cs;

	rx = LC::Util::RegExp { pattern, static_cast<Qt::CaseSensitivity> (cs) };

	return in;
}
