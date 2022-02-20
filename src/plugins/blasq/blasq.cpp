/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "blasq.h"
#include <QIcon>
#include <QtQuick>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <util/util.h>
#include <util/sys/mimedetector.h>
#include <interfaces/entitytesthandleresult.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/irootwindowsmanager.h>
#include "interfaces/blasq/iservicesplugin.h"
#include "interfaces/blasq/iaccount.h"
#include "interfaces/blasq/iservice.h"
#include "interfaces/blasq/isupportuploads.h"
#include "xmlsettingsmanager.h"
#include "accountswidget.h"
#include "servicesmanager.h"
#include "accountsmanager.h"
#include "photostab.h"
#include "defaultimagechooser.h"
#include "enumsproxy.h"
#include "datafilteruploader.h"

namespace LC
{
namespace Blasq
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Util::InstallTranslator ("blasq");

		Proxy_ = proxy;

		ServicesMgr_ = new ServicesManager;
		AccountsMgr_ = new AccountsManager (ServicesMgr_);

		XSD_.reset (new Util::XmlSettingsDialog);
		XSD_->RegisterObject (&XmlSettingsManager::Instance (), "blasqsettings.xml");

		XSD_->SetCustomWidget ("AccountsWidget", new AccountsWidget (ServicesMgr_, AccountsMgr_));

		PhotosTabTC_ = TabClassInfo
		{
			GetUniqueID () + "/Photos",
			tr ("Blasq"),
			tr ("All the photos stored in the cloud"),
			GetIcon (),
			1,
			TFOpenableByRequest | TFSuggestOpening
		};

		qmlRegisterUncreatableType<EnumsProxy> ("org.LC.Blasq", 1, 0, "Blasq",
				"This exports otherwise unavailable Blasq datatypes to QML");
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Blasq";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Blasq";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Client for cloud image storage services like Flickr or Picasa.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	TabClasses_t Plugin::GetTabClasses () const
	{
		return { PhotosTabTC_ };
	}

	void Plugin::TabOpenRequested (const QByteArray& tcId)
	{
		TabOpenRequested (tcId, {});
	}

	void Plugin::RecoverTabs (const QList<TabRecoverInfo>& infos)
	{
		for (const auto& info : infos)
		{
			quint8 version = 0;

			QDataStream in (info.Data_);
			in >> version;
			if (version != 1)
				continue;

			QByteArray tc;
			in >> tc;

			TabOpenRequested (tc, info.DynProperties_, &in);
		}
	}

	bool Plugin::HasSimilarTab (const QByteArray& data, const QList<QByteArray>& existing) const
	{
		return StandardSimilarImpl (data, existing,
				[] (const QByteArray& data)
				{
					QByteArray acc;
					QString coll;
					QDataStream str { data };
					str >> acc >> coll;
					return std::make_tuple (acc, coll);
				});
	}

	QSet<QByteArray> Plugin::GetExpectedPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Blasq.General";
		result << "org.LeechCraft.Blasq.ServicePlugin";
		return result;
	}

	void Plugin::AddPlugin (QObject *plugin)
	{
		if (auto isp = qobject_cast<IServicesPlugin*> (plugin))
			ServicesMgr_->AddPlugin (isp);
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return XSD_;
	}

	ImageServiceInfos_t Plugin::GetServices () const
	{
		ImageServiceInfos_t result;
		for (auto acc : AccountsMgr_->GetAccounts ())
			result.append ({ acc->GetID (), acc->GetName () });
		return result;
	}

	IPendingImgSourceRequest* Plugin::RequestImages (const QByteArray& serviceId)
	{
		return new DefaultImageChooser (AccountsMgr_, Proxy_, serviceId);
	}

	IPendingImgSourceRequest* Plugin::StartDefaultChooser ()
	{
		return new DefaultImageChooser (AccountsMgr_, Proxy_);
	}

	EntityTestHandleResult Plugin::CouldHandle (const Entity& entity) const
	{
		if (entity.Mime_ != "x-leechcraft/data-filter-request")
			return {};

		if (!entity.Entity_.value<QImage> ().isNull ())
			return EntityTestHandleResult { EntityTestHandleResult::PHigh };

		const auto& localFile = entity.Entity_.toUrl ().toLocalFile ();
		if (!QFile::exists (localFile))
			return {};

		if (Util::DetectFileMime (localFile).startsWith ("image/"))
			return EntityTestHandleResult { EntityTestHandleResult::PHigh };

		return {};
	}

	void Plugin::Handle (Entity entity)
	{
		new DataFilterUploader (entity, AccountsMgr_);
	}

	QString Plugin::GetFilterVerb () const
	{
		return tr ("Upload image to cloud");
	}

	QList<IDataFilter::FilterVariant> Plugin::GetFilterVariants (const QVariant&) const
	{
		QList<IDataFilter::FilterVariant> result;
		for (auto acc : AccountsMgr_->GetAccounts ())
		{
			if (!qobject_cast<ISupportUploads*> (acc->GetQObject ()))
				continue;

			const auto accService = acc->GetService ();
			accService->GetServiceIcon ();

			result.append ({
					acc->GetID (),
					acc->GetName () + " (" + accService->GetServiceName () + ")",
					tr ("Upload image to account %1 at %2.")
							.arg (acc->GetName ())
							.arg (accService->GetServiceName ()),
					accService->GetServiceIcon ()
				});
		}
		return result;
	}

	void Plugin::TabOpenRequested (const QByteArray& tcId,
			const DynPropertiesList_t& list, QDataStream *recover)
	{
		if (tcId == PhotosTabTC_.TabClass_)
		{
			auto tab = new PhotosTab (AccountsMgr_, PhotosTabTC_, this, Proxy_);
			for (const auto& prop : list)
				tab->setProperty (prop.first, prop.second);
			GetProxyHolder ()->GetRootWindowsManager ()->AddTab (PhotosTabTC_.VisibleName_, tab);

			if (recover)
				tab->RecoverState (*recover);
		}
		else
			qWarning () << Q_FUNC_INFO
					<< "unknown tab class"
					<< tcId;
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_blasq, LC::Blasq::Plugin);
