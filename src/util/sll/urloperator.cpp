/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "urloperator.h"

namespace LC
{
namespace Util
{
	UrlOperator::UrlOperator (QUrl& url)
	: Url_ (url)
	, Query_ (url)
	{
	}

	UrlOperator::~UrlOperator ()
	{
		Flush ();
	}

	void UrlOperator::Flush ()
	{
		Url_.setQuery (Query_);
	}

	UrlOperator& UrlOperator::operator() (const QString& key, const QString& value)
	{
		Query_.addQueryItem (key, value);
		return *this;
	}

	UrlOperator& UrlOperator::operator() (const QString& key, const QByteArray& value)
	{
		return (*this) (key, QString::fromUtf8 (value));
	}

	UrlOperator& UrlOperator::operator() (const QString& key, const char *value)
	{
		return (*this) (key, QString::fromLatin1 (value));
	}

	UrlOperator& UrlOperator::operator() (const QString& key, int value)
	{
		return (*this) (key, QString::number (value));
	}

	UrlOperator& UrlOperator::operator-= (const QString& key)
	{
		Query_.removeQueryItem (key);
		return *this;
	}

	QUrl UrlOperator::operator() ()
	{
		Flush ();
		return Url_;
	}
}
}
