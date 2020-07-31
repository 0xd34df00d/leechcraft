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
#include "interfaces/lmp/ifilterconfigurator.h"
#include "engine/rgfilter.h"

namespace LC
{
namespace LMP
{
	class Path;
	class AudioSource;
	class RGFilter;
	class FilterSettingsManager;

	class RGFilterController : public QObject
							 , public IFilterConfigurator
	{
		Q_OBJECT

		RGFilter * const RGFilter_;
		Path * const Path_;
		FilterSettingsManager * const FSM_;
	public:
		RGFilterController (RGFilter*, Path*);

		void OpenDialog ();
	private slots:
		void setRG ();
		void updateRGData (const AudioSource&);
	};
}
}
