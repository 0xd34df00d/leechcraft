/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "lackmanutil.h"
#include <optional>
#include <QString>
#include <QFileInfo>
#include <QDir>
#include <QCoreApplication>
#include <QAbstractItemModel>
#include <QMainWindow>
#include <QMessageBox>
#include <QtDebug>
#include <util/sll/prelude.h>
#include <util/sll/visitor.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/irootwindowsmanager.h>
#include "repoinfo.h"
#include "packagesmodel.h"

namespace LC
{
namespace LackMan
{
namespace LackManUtil
{
	QString NormalizePackageName (const QString& packageName)
	{
		QString normalized = packageName.simplified ();
		normalized.remove (' ');
		normalized.remove ('\t');
		return normalized;
	}

	InstalledDependencyInfoList GetSystemInstalledPackages (const QString& defVersion)
	{
		InstalledDependencyInfoList result;

		QFileInfoList infoEntries;
#if defined(Q_OS_WIN32)
		infoEntries += QDir (QCoreApplication::applicationDirPath () + "/share/installed")
				.entryInfoList (QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files);
#elif defined(Q_OS_MAC) && !defined(USE_UNIX_LAYOUT)
		infoEntries += QDir (QCoreApplication::applicationDirPath () + "/../installed")
				.entryInfoList (QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files);
#else
		infoEntries += QDir ("/usr/share/leechcraft/installed")
				.entryInfoList (QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files);
		infoEntries += QDir ("/usr/local/share/leechcraft/installed")
				.entryInfoList (QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files);
#endif

		QStringList entries;

		for (const auto& info : infoEntries)
		{
			const auto& path = info.absoluteFilePath ();
			if (info.isFile ())
				entries << path;
			else if (info.isDir ())
				for (const auto& subInfo : QDir { path }.entryInfoList (QDir::Files))
					entries << subInfo.absoluteFilePath ();
		}

		QString nameStart ("Name: ");
		QString versionStart ("Version: ");

		for (const auto& entry : entries)
		{
			QFile file { entry };
			if (!file.open (QIODevice::ReadOnly))
			{
				qWarning () << Q_FUNC_INFO
						<< "unable to open"
						<< entry
						<< "for reading, skipping it.";
				continue;
			}

			InstalledDependencyInfo info;
			info.Source_ = InstalledDependencyInfo::SSystem;

			const auto& lines = QString { file.readAll () }.split ('\n', Qt::SkipEmptyParts);
			for (const auto& untrimmed : lines)
			{
				const auto& string = untrimmed.trimmed ();
				if (string.startsWith (nameStart))
					info.Dep_.Name_ = string.mid (nameStart.length ());
				else if (string.startsWith (versionStart))
					info.Dep_.Version_ = string.mid (versionStart.length ());
			}

			if (info.Dep_.Version_.isEmpty ())
			{
				qWarning () << Q_FUNC_INFO
						<< "dependency version for"
						<< info.Dep_.Name_
						<< "not filled, defaulting to"
						<< defVersion;
				info.Dep_.Version_ = defVersion;
			}

			result << info;
		}

		return result;
	}

	namespace
	{
		template<typename F>
		QList<MarkingError_t> CollectMarkingErrors (const QAbstractItemModel& packages, F&& mark)
		{
			QList<MarkingError_t> errors;
			for (int i = 0, rc = packages.rowCount (); i < rc; ++i)
				if (const auto error = mark (packages.index (i, 0)))
					errors << *error;
			return errors;
		}

		QString DescribeMarkingError (const MarkingError_t& error)
		{
			return Util::Visit (error,
					[] (const UnsatisfiedDependencies& e)
					{
						return QObject::tr ("Dependencies of %1 cannot be satisfied: %2.")
								.arg (e.Package_, e.Dependencies_.join (", "));
					},
					[] (const PackageLookupFailed& e)
					{
						return QObject::tr ("Package %1 could not be looked up: %2.")
								.arg (QString::number (e.PackageId_), e.What_);
					});
		}
	}

	QList<MarkingError_t> MarkAllForUpgrade (const QAbstractItemModel& packages, PendingManager& pm)
	{
		const auto& pending = pm.GetPendingUpdate ();
		return CollectMarkingErrors (packages,
				[&] (const QModelIndex& idx) -> std::optional<MarkingError_t>
				{
					const auto id = idx.data (PackagesModel::PMRPackageID).toInt ();
					if (!idx.data (PackagesModel::PMRUpgradable).toBool () || pending.contains (id))
						return {};
					return pm.ToggleUpdate (id, true).MaybeLeft ();
				});
	}

	void ReportMarkingErrors (const QList<MarkingError_t>& errors)
	{
		if (errors.isEmpty ())
			return;

		QMessageBox::warning (GetProxyHolder ()->GetRootWindowsManager ()->GetPreferredWindow (),
				QObject::tr ("Unable to mark %n package(s)", nullptr, errors.size ()),
				Util::Map (errors, &DescribeMarkingError).join ("\n"));
	}
}
}
}
