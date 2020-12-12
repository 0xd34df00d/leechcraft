/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "cpufeatures.h"
#include <mutex>
#include <QStringList>
#include <QtDebug>

#if defined (Q_PROCESSOR_X86)
#define HAS_CPUID
#endif

#ifdef HAS_CPUID
#include <cpuid.h>
#endif

#include <util/sll/unreachable.h>

namespace LC
{
namespace Util
{
	CpuFeatures::CpuFeatures ()
	{
#ifdef HAS_CPUID
		uint32_t eax = 0, ebx = 0, ecx = 0, edx = 0;
		if (!__get_cpuid (1, &eax, &ebx, &ecx, &edx))
			qWarning () << Q_FUNC_INFO
					<< "failed to get CPUID eax = 1";
		else
			Ecx1_ = ecx;

		if (__get_cpuid_max (0, nullptr) < 7)
			qWarning () << Q_FUNC_INFO
					<< "cpuid max less than 7";
		else
		{
			__cpuid_count (7, 0, eax, ebx, ecx, edx);
			Ebx7_ = ebx;
		}
#endif

		static std::once_flag dbgFlag;
		std::call_once (dbgFlag,
				[this] { DumpDetectedFeatures (); });
	}

	QString CpuFeatures::GetFeatureName (Feature feature)
	{
		switch (feature)
		{
		case Feature::SSSE3:
			return "ssse3";
		case Feature::SSE41:
			return "sse4.1";
		case Feature::AVX:
			return "avx";
		case Feature::XSave:
			return "xsave";
		case Feature::AVX2:
			return "avx2";
		case Feature::None:
			return "";
		}

		Util::Unreachable ();
	}

	bool CpuFeatures::HasFeature (Feature feature) const
	{
		switch (feature)
		{
		case Feature::SSSE3:
			return Ecx1_ & (1 << 9);
		case Feature::SSE41:
			return Ecx1_ & (1 << 19);
		case Feature::AVX:
			return Ecx1_ & (1 << 28);
		case Feature::XSave:
			return Ecx1_ & (1 << 26);
		case Feature::AVX2:
			return HasFeature (Feature::XSave) && (Ebx7_ & (1 << 5));
		case Feature::None:
			return true;
		}

		Util::Unreachable ();
	}

	void CpuFeatures::DumpDetectedFeatures () const
	{
		if (qgetenv ("DUMP_CPUFEATURES").isEmpty ())
			return;

		QStringList detected;
		QStringList undetected;

		for (int i = 0; i < static_cast<int> (Feature::None); ++i)
		{
			const auto feature = static_cast<Feature> (i);
			const auto& featureName = GetFeatureName (feature);
			if (HasFeature (feature))
				detected << featureName;
			else
				undetected << featureName;
		}

		qDebug () << Q_FUNC_INFO;
		qDebug () << "detected the following CPU features:" << detected.join (' ').toUtf8 ().constData ();
		qDebug () << "couldn't detect the following CPU features:" << undetected.join (' ').toUtf8 ().constData ();
	}
}
}
