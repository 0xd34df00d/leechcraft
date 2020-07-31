/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef INTERFACES_EXCEPTIONS_H
#define INTERFACES_EXCEPTIONS_H
#include <exception>
#include <QString>
#include <QList>
#include "utilconfig.h"

class QObject;

namespace LC
{
	class UTIL_API StandardException : public std::exception
	{
	protected:
		QString What_;
	public:
		StandardException (const QString& what);
		virtual ~StandardException () throw ();

		virtual const char* what () const throw ();
	};

	class UTIL_API DependencyException : public StandardException
	{
	public:
		DependencyException (const QString& what);
		virtual ~DependencyException () throw ();
	};

	class UTIL_API InjectionFailureException : public DependencyException
	{
	public:
		InjectionFailureException (const QString& what);
		virtual ~InjectionFailureException () throw ();
	};

	class UTIL_API ReleaseFailureException : public DependencyException
	{
		QList<QObject*> Holders_;
	public:
		ReleaseFailureException (const QString& what,
				const QList<QObject*>& holders);
		virtual ~ReleaseFailureException () throw ();

		virtual const char* what () const throw ();
	};

	class UTIL_API SerializationException : public StandardException
	{
	public:
		SerializationException (const QString& what);
		virtual ~SerializationException () throw ();
	};

	class UTIL_API UnknownVersionException : public SerializationException
	{
	protected:
		qint64 Version_;
	public:
		UnknownVersionException (qint64 version, const QString& what);
		virtual ~UnknownVersionException () throw ();

		virtual const char* what () const throw ();
	};
};

#endif
