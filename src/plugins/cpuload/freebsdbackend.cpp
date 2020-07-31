/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "freebsdbackend.h"
#include <memory>
#include <fcntl.h>
#include <kvm.h>
#include <sys/param.h>
#include <sys/pcpu.h>
#include <sys/sysctl.h>
#include <QtDebug>

namespace LC
{
namespace CpuLoad
{
	void FreeBSDBackend::Update ()
	{
		const auto kvm = kvm_open (nullptr, nullptr, nullptr, O_RDONLY, "LeechCraft CpuLoad"); // TODO proper error reporting
		if (!kvm)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to open kernel memory";
			return;
		}

		const auto cpuCount = kvm_getncpus (kvm);
		for (int i = 0; i < cpuCount; ++i)
		{
			std::unique_ptr<void, decltype (&free)> rawPcpu { kvm_getpcpu (kvm, i), &free };
			auto pcp = static_cast<pcpu*> (rawPcpu.get ());
		}

		kvm_close (kvm);
	}

	int FreeBSDBackend::GetCpuCount () const
	{
		return 0;
	}

	QMap<LoadPriority, LoadTypeInfo> FreeBSDBackend::GetLoads (int cpu) const
	{
		return {};
	}
}
}
