/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include "abbreviation.h"

namespace LC::Azoth::Abbrev
{
	class AbbrevsManager : public QObject
	{
		Q_OBJECT

		QList<Abbreviation> Abbrevs_;
	public:
		AbbrevsManager (QObject* = nullptr);

		void Add (const Abbreviation&);
		const QList<Abbreviation>& List () const;
		void Remove (int);

		QString Process (QString) const;
	private:
		void Load ();
		void Save () const;
	};
}
