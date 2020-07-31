/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once
#include "bandinfo.h"

class QByteArray;

class QStringList;

namespace LC
{
namespace LMP
{
namespace Fradj
{
	class IEqualizer
	{
	public:
		virtual ~IEqualizer () {}

		virtual QByteArray GetEffectId () const = 0;

		virtual BandInfos_t GetFixedBands () const = 0;

		virtual QStringList GetPresets () const = 0;
		virtual void SetPreset (const QString&) = 0;

		virtual QList<double> GetGains () const = 0;
		virtual void SetGains (const QList<double>& gains) = 0;
	};
}
}
}
