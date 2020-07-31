/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "exceptions.h"
#include <typeinfo>
#include <QDebug>

namespace LC
{
	StandardException::StandardException (const QString& what)
	: What_ (what)
	{
	}

	StandardException::~StandardException () throw ()
	{
	}

	const char* StandardException::what () const throw ()
	{
		return qPrintable (QString ("StandardException (") +
				typeid (*this).name () + "): " +
				What_);
	}

	DependencyException::DependencyException (const QString& de)
	: StandardException (de)
	{
	}

	DependencyException::~DependencyException () throw ()
	{
	}

	InjectionFailureException::InjectionFailureException (const QString& what)
	: DependencyException (what)
	{
	}

	InjectionFailureException::~InjectionFailureException () throw ()
	{
	}

	ReleaseFailureException::ReleaseFailureException (const QString& what,
			const QList<QObject*>& holders)
	: DependencyException (what)
	, Holders_ (holders)
	{
	}

	ReleaseFailureException::~ReleaseFailureException () throw ()
	{
	}

	const char* ReleaseFailureException::what () const throw ()
	{
		QString out;
		QDebug debug (&out);
		debug << What_ << "; holders:" << Holders_;
		return qPrintable (out);
	}

	SerializationException::SerializationException (const QString& what)
	: StandardException (what)
	{
	}

	SerializationException::~SerializationException () throw ()
	{
	}

	UnknownVersionException::UnknownVersionException (qint64 version, const QString& what)
	: SerializationException (what)
	, Version_ (version)
	{
	}

	UnknownVersionException::~UnknownVersionException () throw ()
	{
	}

	const char* UnknownVersionException::what () const throw ()
	{
		QString out;
		QDebug debug (&out);
		debug << "UnknownVersionException: version"
				<< Version_
				<< ";"
				<< What_;
		return qPrintable (out);
	}
};
