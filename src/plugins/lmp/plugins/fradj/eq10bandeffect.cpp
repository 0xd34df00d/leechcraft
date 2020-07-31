/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "eq10bandeffect.h"
#include <QStringList>
#include <QtDebug>
#include <gst/gst.h>
#include "eqconfigurator.h"

namespace LC
{
namespace LMP
{
namespace Fradj
{
	Eq10BandEffect::Eq10BandEffect (const QByteArray& filterId)
	: FilterId_ { filterId }
	, Equalizer_ { gst_element_factory_make ("equalizer-10bands", nullptr) }
	, Configurator_ { new EqConfigurator { this, this } }
	{
		Configurator_->Restore ();
	}

	QByteArray Eq10BandEffect::GetEffectId () const
	{
		return FilterId_;
	}

	QByteArray Eq10BandEffect::GetInstanceId () const
	{
		return FilterId_;
	}

	IFilterConfigurator* Eq10BandEffect::GetConfigurator () const
	{
		return Configurator_;
	}

	BandInfos_t Eq10BandEffect::GetFixedBands () const
	{
		return
		{
			{ 29 },
			{ 59 },
			{ 119 },
			{ 237 },
			{ 474 },
			{ 947 },
			{ 1889 },
			{ 3770 },
			{ 7523 },
			{ 15011 }
		};
	}

	QStringList Eq10BandEffect::GetPresets () const
	{
		QStringList result;
		auto presets = gst_preset_get_preset_names (GST_PRESET (Equalizer_));
		for (auto p = presets; *p; ++p)
			result << QString::fromUtf8 (*p);
		g_strfreev (presets);

		result.sort ();

		return result;
	}

	void Eq10BandEffect::SetPreset (const QString& preset)
	{
		gst_preset_load_preset (GST_PRESET (Equalizer_), preset.toUtf8 ().constData ());
	}

	QList<double> Eq10BandEffect::GetGains () const
	{
		QList<double> result;

		for (int i = 0, size = GetFixedBands ().size (); i < size; ++i)
		{
			const auto& optName = "band" + QByteArray::number (i);
			gdouble value = 0;
			g_object_get (Equalizer_, optName.constData (), &value, nullptr);

			result << value;
		}

		return result;
	}

	void Eq10BandEffect::SetGains (const QList<double>& gains)
	{
		if (gains.size () != GetFixedBands ().size ())
		{
			qWarning () << Q_FUNC_INFO
					<< "unexpected gains count:"
					<< gains;
			return;
		}

		for (int i = 0; i < gains.size (); ++i)
		{
			const auto& optName = "band" + QByteArray::number (i);
			g_object_set (Equalizer_,
					optName.constData (),
					static_cast<gdouble> (gains.at (i)),
					nullptr);
		}
	}

	GstElement* Eq10BandEffect::GetElement () const
	{
		return Equalizer_;
	}
}
}
}
