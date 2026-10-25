/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QList>
#include "pendingmanager.h"

class QAbstractItemModel;
class QString;

namespace LC
{
namespace LackMan
{
struct InstalledDependencyInfo;
using InstalledDependencyInfoList = QList<InstalledDependencyInfo>;

namespace LackManUtil
{
	QString NormalizePackageName (const QString&);

	InstalledDependencyInfoList GetSystemInstalledPackages (const QString& defVersion);

	QList<MarkingError_t> MarkAllForUpgrade (const QAbstractItemModel& packages, PendingManager&);
	void ReportMarkingErrors (const QList<MarkingError_t>&);
}
}
}
