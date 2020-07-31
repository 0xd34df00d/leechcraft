/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "processgraphbuilder.h"
#include <algorithm>
#include <optional>
#include <QDir>
#include <QMap>
#include <QtDebug>
#include <QStandardItemModel>

namespace LC
{
namespace Eleeminator
{
	namespace
	{
		std::optional<int> GetParentPid (QDir subdir, const QString& subdirName)
		{
			if (!subdir.cd (subdirName))
				return {};

			QFile file { subdir.filePath ("status") };
			if (!file.open (QIODevice::ReadOnly))
				return {};

			static QByteArray marker { "PPid:" };

			QByteArray line;
			do
			{
				line = file.readLine ();
				if (!line.startsWith (marker))
					continue;

				const auto& parentPidStr = line.mid (marker.size ()).trimmed ();
				bool ok = false;
				const auto parentPid = parentPidStr.toInt (&ok);
				return ok ? parentPid : std::optional<int> {};
			}
			while (!line.isEmpty ());

			return {};
		}

		typedef QMap<int, QList<int>> RawProcessGraph_t;

		RawProcessGraph_t BuildRawProcessGraph ()
		{
			RawProcessGraph_t processGraph;

			const QDir procDir { "/proc" };
			if (!procDir.isReadable ())
				return {};

			for (const auto& subdirName : procDir.entryList (QDir::Dirs | QDir::NoDotAndDotDot))
			{
				bool ok = false;
				const auto processPid = subdirName.toInt (&ok);
				if (!ok)
					continue;

				if (const auto& parentPid = GetParentPid (procDir, subdirName))
					processGraph [*parentPid] << processPid;
			}

			return processGraph;
		}

		void FillProcessInfo (ProcessInfo& info)
		{
			QDir processDir { "/proc/" + QString::number (info.Pid_) };
			if (!processDir.isReadable ())
				return;

			QFile commFile { processDir.filePath ("comm") };
			if (commFile.open (QIODevice::ReadOnly))
				info.Command_ = QString::fromUtf8 (commFile.readAll ().trimmed ());

			QFile cmdLineFile { processDir.filePath ("cmdline") };
			if (cmdLineFile.open (QIODevice::ReadOnly))
			{
				auto cmdLine = cmdLineFile.readAll ().trimmed ();
				cmdLine.replace ('\0', ' ');
				cmdLine.append ('\0');
				info.CommandLine_ = QString::fromUtf8 (cmdLine).trimmed ();
			}
		}

		ProcessInfo Convert2Info (const RawProcessGraph_t& rawProcessGraph, int pid)
		{
			ProcessInfo result { pid, "(UNKNOWN)", "(UNKNOWN)", {} };

			FillProcessInfo (result);

			auto childPids = rawProcessGraph.value (pid);
			std::sort (childPids.begin (), childPids.end ());
			for (const auto childPid : childPids)
				result.Children_ << Convert2Info (rawProcessGraph, childPid);

			return result;
		}
	}

	ProcessGraphBuilder::ProcessGraphBuilder (int rootPid)
	: Root_ (Convert2Info (BuildRawProcessGraph (), rootPid))
	{
	}

	ProcessInfo ProcessGraphBuilder::GetProcessTree () const
	{
		return Root_;
	}

	bool ProcessGraphBuilder::IsEmpty () const
	{
		return Root_.Children_.isEmpty ();
	}

	namespace
	{
		void AppendInfoRow (const ProcessInfo& info, QStandardItem *parent)
		{
			QList<QStandardItem*> row
			{
				new QStandardItem { QString::number (info.Pid_) },
				new QStandardItem { info.Command_ },
				new QStandardItem { info.CommandLine_ }
			};
			for (auto item : row)
				item->setEditable (false);

			for (const auto& child : info.Children_)
				AppendInfoRow (child, row.first ());

			parent->appendRow (row);
		}
	}

	QAbstractItemModel* ProcessGraphBuilder::CreateModel () const
	{
		auto model = new QStandardItemModel;
		model->setHorizontalHeaderLabels ({ QObject::tr ("PID"), QObject::tr ("Command"), QObject::tr ("Arguments") });

		for (const auto& child : Root_.Children_)
			AppendInfoRow (child, model->invisibleRootItem ());

		return model;
	}
}
}
