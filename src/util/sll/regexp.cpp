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

namespace LC::Util
{
	namespace
	{
		struct RegExpRegisterGuard
		{
			RegExpRegisterGuard ()
			{
				qRegisterMetaType<RegExp> ("Util::RegExp");
			}
		} Guard;
	}

	bool RegExp::IsFast ()
	{
		return true;
	}

	RegExp::RegExp (const QString& str, Qt::CaseSensitivity cs)
	: Rx_ { str }
	{
		if (cs == Qt::CaseInsensitive)
			Rx_.setPatternOptions (QRegularExpression::CaseInsensitiveOption);
	}

	bool RegExp::Matches (const QString& str) const
	{
		return Rx_.match (str).hasMatch ();
	}

	bool RegExp::Matches (const QByteArray& ba) const
	{
		return Rx_.match (ba).hasMatch ();
	}

	QString RegExp::GetPattern () const
	{
		return Rx_.pattern ();
	}

	Qt::CaseSensitivity RegExp::GetCaseSensitivity () const
	{
		return Rx_.patternOptions () & QRegularExpression::CaseInsensitiveOption ?
				Qt::CaseInsensitive :
				Qt::CaseSensitive;
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
