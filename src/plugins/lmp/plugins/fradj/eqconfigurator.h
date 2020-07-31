/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/lmp/ifilterconfigurator.h>
#include "bandinfo.h"

namespace LC
{
namespace LMP
{
namespace Fradj
{
	class IEqualizer;
	class EqConfiguratorDialog;

	class EqConfigurator : public QObject
						 , public IFilterConfigurator
	{
		IEqualizer * const IEq_;
		const QByteArray ID_;
		BandInfos_t Bands_;
	public:
		EqConfigurator (IEqualizer*, QObject*);

		void Restore ();

		void OpenDialog ();
	private:
		QList<double> ReadGains () const;
		void SaveGains (const QList<double>&) const;
	};
}
}
}
