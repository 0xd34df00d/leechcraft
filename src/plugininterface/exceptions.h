/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2010  Georg Rudoy
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

#ifndef INTERFACES_EXCEPTIONS_H
#define INTERFACES_EXCEPTIONS_H
#include <exception>
#include <QString>
#include <QList>
#include "piconfig.h"

class QObject;

namespace LeechCraft
{
	class PLUGININTERFACE_API StandardException : public std::exception
	{
	protected:
		QString What_;
	public:
		StandardException (const QString& what);
		virtual ~StandardException () throw ();

		virtual const char* what () const throw ();
	};

	class PLUGININTERFACE_API DependencyException : public StandardException
	{
	public:
		DependencyException (const QString& what);
		virtual ~DependencyException () throw ();
	};

	class PLUGININTERFACE_API InjectionFailureException : public DependencyException
	{
	public:
		InjectionFailureException (const QString& what);
		virtual ~InjectionFailureException () throw ();
	};

	class PLUGININTERFACE_API ReleaseFailureException : public DependencyException
	{
		QList<QObject*> Holders_;
	public:
		ReleaseFailureException (const QString& what,
				const QList<QObject*>& holders);
		virtual ~ReleaseFailureException () throw ();

		virtual const char* what () const throw ();
	};

	class PLUGININTERFACE_API SerializationException : public StandardException
	{
	public:
		SerializationException (const QString& what);
		virtual ~SerializationException () throw ();
	};

	class PLUGININTERFACE_API UnknownVersionException : public SerializationException
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
