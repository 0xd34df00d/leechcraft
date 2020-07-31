/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "linuxbackend.h"
#include <cmath>
#include <QFile>
#include <QtDebug>

namespace LC
{
namespace CpuLoad
{
	LinuxBackend::LinuxBackend (QObject *parent)
	: Backend { parent }
	{
	}

	namespace
	{
		Cummulative_t ReadProcStat ()
		{
			QFile file { "/proc/stat" };
			if (!file.open (QIODevice::ReadOnly))
			{
				qWarning () << Q_FUNC_INFO
						<< "cannot open"
						<< file.fileName ()
						<< file.errorString ();
				return {};
			}

			static const QByteArray cpuMarker { "cpu" };

			Cummulative_t result;

			for (const auto& line : file.readAll ().split ('\n'))
			{
				const auto& elems = line.split (' ');
				const auto& id = elems.value (0);
				if (!id.startsWith (cpuMarker))
					continue;

				bool ok = true;
				const auto cpuIdx = id.mid (cpuMarker.size ()).toInt (&ok);
				if (!ok)
					continue;

				QVector<long> cpuVec;
				for (const auto& elem : elems.mid (1))
				{
					bool ok = false;
					const auto num = elem.toLong (&ok);
					if (ok)
						cpuVec << num;
				}

				if (result.size () <= cpuIdx)
					result.resize (cpuIdx + 1);
				result [cpuIdx] = cpuVec;
			}

			return result;
		}
	}

	void LinuxBackend::Update ()
	{
		const int prevCpuCount = GetCpuCount ();

		auto savedLast = ReadProcStat ();
		std::swap (savedLast, LastCummulative_);

		const auto curCpuCount = GetCpuCount ();
		if (curCpuCount != prevCpuCount)
		{
			emit cpuCountChanged (curCpuCount);
			return;
		}

		Loads_.clear ();
		Loads_.resize (LastCummulative_.size ());

		for (int i = 0; i < curCpuCount; ++i)
		{
			auto& cpuLoad = Loads_ [i];

			auto lastCpuStats = LastCummulative_ [i];
			const auto& prevCpuStats = savedLast [i];
			for (int j = 0; j < lastCpuStats.size (); ++j)
				lastCpuStats [j] -= prevCpuStats [j];

			const auto total = lastCpuStats [0] + lastCpuStats [1] + lastCpuStats [2] + lastCpuStats [3] + lastCpuStats [4];

			auto setLoadPart = [&cpuLoad, total, &lastCpuStats] (int idx, LoadPriority prio)
			{
				const auto thisLoad = static_cast<double> (lastCpuStats [idx]) / total;
				cpuLoad [prio] = LoadTypeInfo { thisLoad };
			};

			setLoadPart (4, LoadPriority::IO);
			setLoadPart (1, LoadPriority::Low);
			setLoadPart (0, LoadPriority::Medium);
			setLoadPart (2, LoadPriority::High);
		}
	}

	int LinuxBackend::GetCpuCount () const
	{
		return LastCummulative_.size ();
	}

	QMap<LoadPriority, LoadTypeInfo> LinuxBackend::GetLoads (int cpu) const
	{
		return Loads_.value (cpu);
	}

}
}
