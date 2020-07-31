/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "rgfilter.h"
#include <QStringList>
#include "util/lmp/gstutil.h"
#include "../gstfix.h"
#include "rgfiltercontroller.h"

namespace LC
{
namespace LMP
{
	RGFilter::RGFilter (Path *path)
	: Elem_ (gst_bin_new ("rgbin"))
	, TagInject_ (gst_element_factory_make ("taginject", "taginject"))
	, RGVol_ (gst_element_factory_make ("rgvolume", "rgvol"))
	, RGLimiter_ (gst_element_factory_make ("rglimiter", "rglim"))
	, Controller_ (new RGFilterController (this, path))
	{
		const auto convIn = gst_element_factory_make ("audioconvert", "convIn");
		const auto convOut = gst_element_factory_make ("audioconvert", "convOut");

		gst_bin_add_many (GST_BIN (Elem_), TagInject_, RGVol_, RGLimiter_, convIn, convOut, nullptr);
		gst_element_link_many (convIn,
				TagInject_,
				RGVol_,
				RGLimiter_,
				convOut,
				nullptr);

		GstUtil::AddGhostPad (convIn, Elem_, "sink");
		GstUtil::AddGhostPad (convOut, Elem_, "src");
	}

	QByteArray RGFilter::GetEffectId () const
	{
		return "org.LeechCraft.LMP.RG";
	}

	QByteArray RGFilter::GetInstanceId () const
	{
		return GetEffectId ();
	}

	IFilterConfigurator* RGFilter::GetConfigurator () const
	{
		return Controller_.get ();
	}

	void RGFilter::SetRG (const RGData& data)
	{
		QStringList pairs;
		auto addPair = [&pairs] (const QString& tagName, double value)
		{
			pairs += tagName + '=' + QString::number (value, 'f', 2);
		};

		addPair (GST_TAG_TRACK_GAIN, data.TrackGain_);
		addPair (GST_TAG_TRACK_PEAK, data.TrackPeak_);
		addPair (GST_TAG_ALBUM_GAIN, data.AlbumGain_);
		addPair (GST_TAG_ALBUM_PEAK, data.AlbumPeak_);
		addPair (GST_TAG_REFERENCE_LEVEL, 89);

		g_object_set (TagInject_, "tags", pairs.join (",").toUtf8 ().constData (), nullptr);
	}

	void RGFilter::SetAlbumMode (bool albumMode)
	{
		g_object_set (RGVol_, "album-mode", static_cast<gboolean> (albumMode), nullptr);
	}

	void RGFilter::SetLimiterEnabled (bool enabled)
	{
		g_object_set (RGLimiter_, "enabled", static_cast<gboolean> (enabled), nullptr);
	}

	void RGFilter::SetPreamp (double preamp)
	{
		g_object_set (RGVol_, "pre-amp", static_cast<gdouble> (preamp), nullptr);
	}

	GstElement* RGFilter::GetElement () const
	{
		return Elem_;
	}
}
}
