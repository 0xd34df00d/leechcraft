/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "lmsensorsbackend.h"
#include <QtDebug>
#include <sensors/sensors.h>
#include <util/sll/qtutil.h>

namespace LC::HotSensors
{
	struct StoredChipName
	{
		QByteArray Prefix_;
		sensors_bus_id Bus_ = {};
		int Addr_ = 0;
		QByteArray Path_;

		StoredChipName () = default;
		explicit StoredChipName (const sensors_chip_name*);

		sensors_chip_name ToSensorsChip ();
	};

	struct StoredSubfeature
	{
		StoredChipName Chip_;
		int SF_;
	};

	struct StoredTemp
	{
		double Max_;
		double Crit_;
		StoredSubfeature SF_;
		QString Name_;
	};

	StoredChipName::StoredChipName (const sensors_chip_name *chipName)
	: Prefix_ { chipName->prefix }
	, Bus_ { chipName->bus }
	, Addr_ { chipName->addr }
	, Path_ { chipName->path }
	{
	}

	sensors_chip_name StoredChipName::ToSensorsChip ()
	{
		return { Prefix_.data (), Bus_, Addr_, Path_.data () };
	}

	LmSensorsBackend::LmSensorsBackend (QObject *parent)
	: Backend { parent }
	{
		sensors_init (nullptr);

		EnumerateSensors ();
	}

	LmSensorsBackend::~LmSensorsBackend ()
	{
		sensors_cleanup ();
	}

	void LmSensorsBackend::EnumerateSensors ()
	{
		int nr = 0;
		const sensors_chip_name *chipName = nullptr;
		while ((chipName = sensors_get_detected_chips (nullptr, &nr)))
		{
			int fnr = 0;
			const sensors_feature *feature = nullptr;
			while ((feature = sensors_get_features (chipName, &fnr)))
			{
				if (feature->type != SENSORS_FEATURE_TEMP)
					continue;

				int sfnr = 0;
				const sensors_subfeature *subfeature = nullptr;

				StoredTemp temp
				{
					100,
					100,
					{},
					"%1/%2"_qs.arg (chipName->prefix, sensors_get_label (chipName, feature)),
				};
				while ((subfeature = sensors_get_all_subfeatures (chipName, feature, &sfnr)))
				{
					switch (subfeature->type)
					{
					case SENSORS_SUBFEATURE_TEMP_MAX:
						sensors_get_value (chipName, subfeature->number, &temp.Max_);
						break;
					case SENSORS_SUBFEATURE_TEMP_CRIT:
						sensors_get_value (chipName, subfeature->number, &temp.Crit_);
						break;
					case SENSORS_SUBFEATURE_TEMP_INPUT:
						temp.SF_ = StoredSubfeature { StoredChipName { chipName }, subfeature->number };
						break;
					default:
						break;
					}
				}
				Features_ << temp;
			}
		}
	}

	void LmSensorsBackend::update ()
	{
		Readings_t readings { static_cast<Readings_t::capacity_type> (Features_.size ()) };
		for (auto& feature : Features_)
		{
			const auto chipName = feature.SF_.Chip_.ToSensorsChip ();

			double value = 0;
			sensors_get_value (&chipName, feature.SF_.SF_, &value);

			readings.push_back ({ feature.Name_, value, feature.Max_, feature.Crit_ });
		}
		emit gotReadings (readings);
	}
}
