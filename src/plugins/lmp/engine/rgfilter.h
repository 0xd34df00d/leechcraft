/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QObject>
#include "interfaces/lmp/ifilterelement.h"

namespace LC
{
namespace LMP
{
	class Path;
	class RGFilterController;

	struct RGData
	{
		double TrackGain_;
		double TrackPeak_;
		double AlbumGain_;
		double AlbumPeak_;
	};

	class RGFilter : public IFilterElement
	{
		GstElement * const Elem_;
		GstElement * const TagInject_;
		GstElement * const RGVol_;
		GstElement * const RGLimiter_;

		const std::shared_ptr<RGFilterController> Controller_;
	public:
		RGFilter (Path*);

		QByteArray GetEffectId () const;
		QByteArray GetInstanceId () const;
		IFilterConfigurator* GetConfigurator () const;

		void SetRG (const RGData&);

		void SetAlbumMode (bool);
		void SetLimiterEnabled (bool);
		void SetPreamp (double);
	protected:
		GstElement* GetElement () const;
	};
}
}
