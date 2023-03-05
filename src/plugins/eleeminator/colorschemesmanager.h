/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>

namespace LC::Eleeminator
{
	class ColorSchemesManager : public QObject
	{
	public:
		struct Scheme
		{
			QString Name_;
			QString ID_;
		};
	private:
		QVector<Scheme> Schemes_;
	public:
		explicit ColorSchemesManager (QObject *parent = nullptr);

		QVector<Scheme> GetSchemes () const;
	};
}
