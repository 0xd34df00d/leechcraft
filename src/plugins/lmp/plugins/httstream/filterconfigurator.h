/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include "interfaces/lmp/ifilterconfigurator.h"

namespace LC
{
namespace Util
{
class XmlSettingsDialog;
}

namespace LMP
{
class FilterSettingsManager;

namespace HttStream
{
	class HttpStreamFilter;

	class FilterConfigurator : public QObject
							 , public IFilterConfigurator
	{
		Q_OBJECT

		const QString InstanceId_;
		FilterSettingsManager * const FSM_;
		HttpStreamFilter * const Filter_;
	public:
		FilterConfigurator (const QString&, HttpStreamFilter*);

		void OpenDialog ();
	private:
		void FillAddressModel (Util::XmlSettingsDialog*);
	private slots:
		void handleAddressChanged ();
		void handleEncQualityChanged ();
	};
}
}
}
