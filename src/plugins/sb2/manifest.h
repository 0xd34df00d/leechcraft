/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QStringList>
#include <QIcon>

namespace LC
{
namespace SB2
{
	class Manifest
	{
		const QString QuarkPath_;

		QString ID_;
		QString Name_;
		QIcon Icon_;
		QString Description_;
		QStringList Areas_;
		bool IsHiddenByDefault_ = true;
	public:
		Manifest (const QString&);

		QString GetID () const;
		QString GetName () const;
		QIcon GetIcon () const;
		QString GetDescription () const;
		QStringList GetAreas () const;
		bool IsHiddenByDefault () const;
	private:
		bool TryFullImage (const QString&);
		bool TryTheme (const QString&);
		bool TryLC (const QString&);
	};
}
}
