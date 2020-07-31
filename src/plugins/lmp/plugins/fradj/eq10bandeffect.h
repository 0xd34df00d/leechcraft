/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include "interfaces/lmp/ifilterelement.h"
#include "iequalizer.h"

typedef struct _GstPad GstPad;
typedef struct _GstMessage GstMessage;
typedef struct _GstPadTemplate GstPadTemplate;

namespace LC
{
namespace LMP
{
namespace Fradj
{
	class EqConfigurator;

	class Eq10BandEffect : public QObject
						 , public IFilterElement
						 , public IEqualizer
	{
		const QByteArray FilterId_;

		GstElement * const Equalizer_;

		EqConfigurator * const Configurator_;
	public:
		Eq10BandEffect (const QByteArray& filterId);

		QByteArray GetEffectId () const;
		QByteArray GetInstanceId () const;
		IFilterConfigurator* GetConfigurator () const;

		BandInfos_t GetFixedBands () const;
		QStringList GetPresets () const;
		void SetPreset (const QString&);
		QList<double> GetGains () const;
		void SetGains (const QList<double>&);
	protected:
		GstElement* GetElement () const;
	};
}
}
}

