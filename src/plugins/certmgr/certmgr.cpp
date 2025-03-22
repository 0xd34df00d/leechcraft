/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "certmgr.h"
#include <QIcon>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "xmlsettingsmanager.h"
#include "acceptedrejecteddialog.h"
#include "managerdialog.h"

namespace LC
{
namespace CertMgr
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;

		Manager_.reset (new Manager);

		XSD_.reset (new Util::XmlSettingsDialog);
		XSD_->RegisterObject (&XmlSettingsManager::Instance (), "certmgrsettings.xml");

		connect (XSD_.get (),
				SIGNAL (pushButtonClicked (QString)),
				this,
				SLOT (handleSettingsButton (QString)));
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.CertMgr";
	}

	void Plugin::Release ()
	{
		Manager_.reset ();
	}

	QString Plugin::GetName () const
	{
		return "CertMgr";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("SSL certificates manager.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return XSD_;
	}

	void Plugin::handleSettingsButton (const QString& button)
	{
		if (button == "AcceptedRejected")
		{
			auto dia = new AcceptedRejectedDialog { Proxy_ };
			dia->setAttribute (Qt::WA_DeleteOnClose);
			dia->show ();
		}
		else if (button == "Certificates")
		{
			auto dia = new ManagerDialog { Manager_.get () };
			dia->setAttribute (Qt::WA_DeleteOnClose);
			dia->show ();
		}
		else
			qWarning () << Q_FUNC_INFO
					<< "unknown button"
					<< button;
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_certmgr, LC::CertMgr::Plugin);
