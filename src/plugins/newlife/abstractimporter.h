/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QIcon>

class QWizardPage;

namespace LC
{
struct Entity;

namespace NewLife
{
	class AbstractImporter : public QObject
	{
	public:
		using QObject::QObject;

		virtual QStringList GetIcons () const;
		virtual QStringList GetNames () const = 0;
		virtual QList<QWizardPage*> GetWizardPages () const = 0;
	};
}
}
