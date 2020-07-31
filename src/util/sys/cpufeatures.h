/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <cstdint>
#include <utility>
#include "sysconfig.h"

class QString;

namespace LC
{
namespace Util
{
	class UTIL_SYS_API CpuFeatures
	{
		uint32_t Ecx1_ = 0;
		uint32_t Ebx7_ = 0;
	public:
		CpuFeatures ();

		enum class Feature
		{
			SSSE3,
			SSE41,
			AVX,
			XSave,
			AVX2,

			None
		};

		bool HasFeature (Feature) const;

		static QString GetFeatureName (Feature);

		template<typename T>
		static T Choose (std::initializer_list<std::pair<Feature, T>> funcs, T fallback)
		{
			const CpuFeatures features;
			for (const auto& pair : funcs)
				if (features.HasFeature (pair.first))
					return pair.second;

			return fallback;
		}
	private:
		void DumpDetectedFeatures () const;
	};
}
}
