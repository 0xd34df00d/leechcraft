/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "filterconfigurator.h"
#include <QStringListModel>
#include <QTimer>
#include <QHostAddress>
#include <QtDebug>
#include <util/xsd/util.h>
#include <util/network/addresses.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <util/lmp/filtersettingsmanager.h>
#include "httpstreamfilter.h"

namespace LC
{
namespace LMP
{
namespace HttStream
{
	FilterConfigurator::FilterConfigurator (const QString& instanceId, HttpStreamFilter *parent)
	: QObject { parent }
	, InstanceId_ { instanceId }
	, FSM_ { new FilterSettingsManager { instanceId, this } }
	, Filter_ { parent }
	{
		Util::XmlSettingsDialog dia;
		dia.RegisterObject (FSM_, "lmphttstreamfiltersettings.xml");
		FillAddressModel (&dia);

		FSM_->RegisterObject ("EncQuality", this, "handleEncQualityChanged");
		QTimer::singleShot (0,
				this,
				SLOT (handleEncQualityChanged ()));

		FSM_->RegisterObject ({ "Address", "Port" }, this, "handleAddressChanged");
		QTimer::singleShot (0,
				this,
				SLOT (handleAddressChanged ()));
	}

	void FilterConfigurator::OpenDialog ()
	{
		const auto xsd = Util::OpenXSD (tr ("HTTP streaming settings"), "lmphttstreamfiltersettings.xml", FSM_);
		FillAddressModel (xsd);
	}

	void FilterConfigurator::FillAddressModel (Util::XmlSettingsDialog *xsd)
	{
		QStringList addresses;
		for (const auto& addr : Util::GetAllAddresses ())
			addresses << addr.toString ();
		qDebug () << Q_FUNC_INFO << addresses;
		xsd->SetDataSource ("Address", new QStringListModel { addresses, xsd });
	}

	void FilterConfigurator::handleAddressChanged ()
	{
		const auto& addr = FSM_->property ("Address").toString ();
		const auto port = FSM_->property ("Port").toInt ();
		Filter_->SetAddress (addr, port);
	}

	void FilterConfigurator::handleEncQualityChanged ()
	{
		const auto quality = FSM_->property ("EncQuality").toDouble ();
		Filter_->SetQuality (quality);
	}
}
}
}
