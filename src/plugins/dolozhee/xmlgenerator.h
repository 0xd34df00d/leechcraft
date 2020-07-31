/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QString>
#include "reporttypepage.h"
#include "structures.h"

namespace LC
{
namespace Dolozhee
{
	class XMLGenerator
	{
	public:
		QByteArray RegisterUser (const QString&, const QString&,
				const QString&, const QString&, const QString&) const;
		QByteArray CreateIssue (const QString&, QString,
				int, ReportTypePage::Type, ReportTypePage::Priority,
				const QList<FileInfo>&) const;
	};
}
}
