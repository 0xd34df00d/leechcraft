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

#include "exceptions.h"
#include <QDebug>

namespace LeechCraft
{
	DependencyException::DependencyException (const QString& de)
	: What_ (de)
	{
	}

	DependencyException::~DependencyException () throw ()
	{
	}

	const char* DependencyException::what () const throw ()
	{
		return qPrintable (What_);
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
};
