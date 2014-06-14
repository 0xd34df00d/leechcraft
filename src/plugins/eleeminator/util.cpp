/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2014  Georg Rudoy
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 **********************************************************************/

#include "util.h"
#include <algorithm>
#include <boost/optional.hpp>
#include <QDir>
#include <QMap>
#include <QtDebug>
#include <QStandardItemModel>

namespace LeechCraft
{
namespace Eleeminator
{
	QStringList ListProcessChildren (int pid)
	{
		const QDir procDir { "/proc" };
		if (!procDir.isReadable ())
			return {};

		static QByteArray marker { "PPid:" };

		QStringList result;
		for (const auto& subdirName : procDir.entryList (QDir::Dirs | QDir::NoDotAndDotDot))
		{
			bool ok = false;
			subdirName.toInt (&ok);
			if (!ok)
				continue;

			auto subdir = procDir;
			if (!subdir.cd (subdirName))
				continue;

			QFile file { subdir.filePath ("status") };
			if (!file.open (QIODevice::ReadOnly))
				continue;

			QByteArray line;
			do
			{
				line = file.readLine ();
				if (!line.startsWith (marker))
					continue;

				const auto& parentPidStr = line.mid (marker.size ()).trimmed ();
				const auto parentPid = parentPidStr.toInt (&ok);
				if (ok && parentPid == pid)
				{
					QFile commandFile { subdir.filePath ("comm") };
					if (!commandFile.open (QIODevice::ReadOnly))
						result << "(UNKNOWN)";
					else
						result << QString::fromUtf8 (commandFile.readAll ()).trimmed ();
				}

				break;
			}
			while (!line.isEmpty ());
		}

		return result;
	}

	namespace
	{
		boost::optional<int> GetParentPid (QDir subdir, const QString& subdirName)
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
				return ok ? parentPid : boost::optional<int> {};
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

			QStringList result;
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

	ProcessInfo GetProcessTree (int rootPid)
	{
		const auto& rawGraph = BuildRawProcessGraph ();
		return Convert2Info (rawGraph, rootPid);
	}

	void PrintPI (QDebug& debug, const ProcessInfo& info, int level = 0)
	{
		const int levelShift = 2;

		auto printShift = [&debug, level]
		{
			for (int i = 0; i < level; ++i)
				debug << " ";
		};

		printShift ();
		debug << "PI { Pid: " << info.Pid_ << "; command: " << info.Command_ << "; command line: " << info.CommandLine_ << "; children: " << info.Children_.size ();

		if (!info.Children_.isEmpty ())
		{
			debug << ":\n";
			for (const auto& child : info.Children_)
				PrintPI (debug, child, level + levelShift);
		}

		printShift ();
		debug << "}\n";
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

	QAbstractItemModel* CreateModel (const ProcessInfo& info)
	{
		auto model = new QStandardItemModel;
		model->setHorizontalHeaderLabels ({ QObject::tr ("PID"), QObject::tr ("Command"), QObject::tr ("Arguments") });

		for (const auto& child : info.Children_)
			AppendInfoRow (child, model->invisibleRootItem ());

		return model;
	}
}
}

QDebug operator<< (QDebug debug, const LeechCraft::Eleeminator::ProcessInfo& info)
{
	LeechCraft::Eleeminator::PrintPI (debug.nospace (), info);
	return debug.space ();
}
