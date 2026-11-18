/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "winbackend.h"
#include <utility>
#include <windows.h>
#include <winternl.h>
#include <QtDebug>

extern "C" NTSTATUS NTAPI NtQuerySystemInformationEx (SYSTEM_INFORMATION_CLASS,
		PVOID input, ULONG inputLength, PVOID output, ULONG outputLength, PULONG returnLength);

namespace LC::CpuLoad
{
	void WinBackend::Update ()
	{
		QList<CpuTimes> times;
		for (WORD group = 0, groupsCount = GetActiveProcessorGroupCount (); group < groupsCount; ++group)
		{
			QList<SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION> infos (GetActiveProcessorCount (group));
			ULONG filledSize = 0;
			if (const auto status = NtQuerySystemInformationEx (SystemProcessorPerformanceInformation,
						&group, sizeof (group),
						infos.data (), static_cast<ULONG> (infos.size () * sizeof (infos [0])), &filledSize);
				!NT_SUCCESS (status))
			{
				qWarning () << "unable to query the processor times of group" << group
						<< "status" << Qt::hex << static_cast<ULONG> (status);
				return;
			}
			infos.resize (filledSize / sizeof (infos [0]));

			for (const auto& info : infos)
				times.push_back ({
						.Idle_ = info.IdleTime.QuadPart,
						.Kernel_ = info.KernelTime.QuadPart,
						.User_ = info.UserTime.QuadPart,
					});
		}

		Prev_ = std::exchange (Cur_, std::move (times));
		if (Cur_.size () != Prev_.size ())
			emit cpuCountChanged (GetCpuCount ());
	}

	int WinBackend::GetCpuCount () const
	{
		return Cur_.size ();
	}

	QMap<LoadPriority, LoadTypeInfo> WinBackend::GetLoads (int cpu) const
	{
		if (Prev_.size () != Cur_.size () || cpu >= Cur_.size ())
			return {};

		const auto& prev = Prev_ [cpu];
		const auto& cur = Cur_ [cpu];
		const auto idle = cur.Idle_ - prev.Idle_;
		const auto kernel = cur.Kernel_ - prev.Kernel_;
		const auto user = cur.User_ - prev.User_;

		const auto total = kernel + user;
		const auto share = [total] (qint64 time)
		{
			return LoadTypeInfo { total ? static_cast<double> (time) / total : 0 };
		};
		return
		{
			{ LoadPriority::IO, share (0) },
			{ LoadPriority::Low, share (0) },
			{ LoadPriority::Medium, share (user) },
			{ LoadPriority::High, share (kernel - idle) },
		};
	}
}
