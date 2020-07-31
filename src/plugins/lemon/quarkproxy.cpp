/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "quarkproxy.h"
#include "trafficdialog.h"
#include "xmlsettingsmanager.h"

namespace LC
{
namespace Lemon
{
	QuarkProxy::QuarkProxy (TrafficManager *tm, QObject *parent)
	: QObject { parent }
	, TrafficMgr_ { tm }
	{
		XmlSettingsManager::Instance ().RegisterObject ("DownloadColor",
				this, "downloadGraphColorChanged");
		XmlSettingsManager::Instance ().RegisterObject ("UploadColor",
				this, "uploadGraphColorChanged");
	}

	QColor QuarkProxy::GetDownloadGraphColor () const
	{
		return XmlSettingsManager::Instance ().property ("DownloadColor").value<QColor> ();
	}

	QColor QuarkProxy::GetUploadGraphColor () const
	{
		return XmlSettingsManager::Instance ().property ("UploadColor").value<QColor> ();
	}

	void QuarkProxy::showGraph (const QString& ifaceName)
	{
		if (auto dia = Iface2Dialog_ [ifaceName])
		{
			delete dia;
			return;
		}

		auto dia = new TrafficDialog (ifaceName, TrafficMgr_);
		dia->setAttribute (Qt::WA_DeleteOnClose);
		dia->show ();
		Iface2Dialog_ [ifaceName] = dia;
	}
}
}
