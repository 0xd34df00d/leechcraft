/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>

namespace LC
{
namespace Eleeminator
{
	class ColorSchemesManager : public QObject
	{
		Q_OBJECT
	public:
		struct Scheme
		{
			QString Name_;
			QString ID_;
		};
	private:
		QList<Scheme> Schemes_;
	public:
		explicit ColorSchemesManager (QObject *parent = nullptr);

		QList<Scheme> GetSchemes () const;
	private:
		void LoadKonsoleSchemes ();
		void FilterDuplicates ();
	};
}
}
