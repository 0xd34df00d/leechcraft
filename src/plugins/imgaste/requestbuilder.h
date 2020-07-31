/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QString>

namespace LC::Imgaste
{
	class RequestBuilder
	{
		QByteArray Result_;
		QByteArray Boundary_;
	public:
		RequestBuilder ();

		void AddPair (const QString&, const QString&);
		void AddFile (const QString&, const QString&, const QByteArray&);

		QByteArray Build () const;
		int Size () const;
		QString GetBoundary () const;
	};
}
