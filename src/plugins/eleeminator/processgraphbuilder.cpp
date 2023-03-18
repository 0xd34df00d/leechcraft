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
#include <QStandardItemModel>
#include <util/sll/qtutil.h>

namespace LC::Eleeminator
{
	std::optional<int> GetParentPid (const QString& pidStr)
	{
		QFile file { "/proc/" + pidStr + "/status" };
		if (!file.open (QIODevice::ReadOnly))
			return {};

		static const QByteArray marker { "PPid:" };

		for (auto line = file.readLine (); !line.isEmpty (); line = file.readLine ())
		{
			if (!line.startsWith (marker))
				continue;

			const auto& parentPidStr = line.mid (marker.size ()).trimmed ();
			bool ok = false;
			const auto parentPid = parentPidStr.toInt (&ok);
			return ok ? parentPid : std::optional<int> {};
		}

		return {};
	}

	namespace
	{
		using RawProcessGraph_t = QMap<int, QList<int>>;

		RawProcessGraph_t BuildRawProcessGraph ()
		{
			RawProcessGraph_t processGraph;

			const QDir procDir { "/proc"_qs };
			if (!procDir.isReadable ())
				return {};

			for (const auto& subdirName : procDir.entryList (QDir::Dirs | QDir::NoDotAndDotDot))
			{
				bool ok = false;
				const auto processPid = subdirName.toInt (&ok);
				if (!ok)
					continue;

				if (const auto& parentPid = GetParentPid (subdirName))
					processGraph [*parentPid] << processPid;
			}

			return processGraph;
		}

		void FillProcessInfo (ProcessInfo& info)
		{
			const QDir processDir { "/proc/" + QString::number (info.Pid_) };
			if (!processDir.isReadable ())
				return;

			QFile commFile { processDir.filePath ("comm"_qs) };
			if (commFile.open (QIODevice::ReadOnly))
				info.Command_ = QString::fromUtf8 (commFile.readAll ().trimmed ());

			QFile cmdLineFile { processDir.filePath ("cmdline"_qs) };
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
			const QString unknown = "(UNKNOWN)"_qs;
			ProcessInfo result { pid, unknown, unknown, {} };

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

	std::unique_ptr<QAbstractItemModel> ProcessGraphBuilder::CreateModel () const
	{
		auto model = std::make_unique<QStandardItemModel> ();
		model->setHorizontalHeaderLabels ({ QObject::tr ("PID"), QObject::tr ("Command"), QObject::tr ("Arguments") });

		for (const auto& child : Root_.Children_)
			AppendInfoRow (child, model->invisibleRootItem ());

		return model;
	}
}
