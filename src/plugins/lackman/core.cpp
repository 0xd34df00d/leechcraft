/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "core.h"
#include <stdexcept>
#include <QApplication>
#include <QString>
#include <QStandardItemModel>
#include <QDir>
#include <QTimer>
#include <QtDebug>
#include <util/util.h>
#include <util/sll/prelude.h>
#include <util/sll/qtutil.h>
#include <util/xpc/util.h>
#include <xmlsettingsdialog/datasourceroles.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/core/iiconthememanager.h>
#include "repoinfofetcher.h"
#include "storage.h"
#include "packagesmodel.h"
#include "externalresourcemanager.h"
#include "pendingmanager.h"
#include "packageprocessor.h"
#include "versioncomparator.h"
#include "xmlsettingsmanager.h"
#include "updatesnotificationmanager.h"
#include "lackmanutil.h"

namespace LC
{
namespace LackMan
{
	QMap<Dependency::Relation, Comparator_t> Relation2comparator;

	namespace
	{
		void SendEntity (const Entity& e)
		{
			GetProxyHolder ()->GetEntityManager ()->HandleEntity (e);
		}
	}

	Core::Core ()
	: ExternalResourceManager_ (new ExternalResourceManager (this))
	, Storage_ (new Storage (this))
	, PackagesModel_ (new PackagesModel (this))
	, PendingManager_ (new PendingManager (this))
	, PackageProcessor_ (new PackageProcessor (this))
	, ReposModel_ (new QStandardItemModel (this))
	, UpdatesEnabled_ (true)
	{
		Relation2comparator [Dependency::L] = IsVersionLess;
		Relation2comparator [Dependency::G] = [] (QString l, QString r) { return Relation2comparator [Dependency::L] (r, l); };
		Relation2comparator [Dependency::GE] = [] (QString l, QString r) { return !Relation2comparator [Dependency::L] (l, r); };
		Relation2comparator [Dependency::E] = [] (QString l, QString r) { return r == l; };
		Relation2comparator [Dependency::LE] = [] (QString l, QString r) { return !Relation2comparator [Dependency::G] (l, r); };

		connect (PendingManager_,
				SIGNAL (packageUpdateToggled (int, bool)),
				PackagesModel_,
				SLOT (handlePackageUpdateToggled (int)));
		connect (PendingManager_,
				SIGNAL (packageInstallRemoveToggled (int, bool)),
				PackagesModel_,
				SLOT (handlePackageInstallRemoveToggled (int)));

		connect (Storage_,
				SIGNAL (packageRemoved (int)),
				this,
				SLOT (handlePackageRemoved (int)));
		connect (PackageProcessor_,
				SIGNAL (packageInstallError (int, const QString&)),
				this,
				SLOT (handlePackageInstallError (int, const QString&)));
		connect (PackageProcessor_,
				SIGNAL (packageInstalled (int)),
				this,
				SLOT (handlePackageInstalled (int)));
		connect (PackageProcessor_,
				SIGNAL (packageUpdated (int, int)),
				this,
				SLOT (handlePackageUpdated (int, int)));

		QStandardItem *item = new QStandardItem (tr ("URL"));
		item->setData (DataSources::DataFieldType::Url, DataSources::DataSourceRole::FieldType);
		ReposModel_->setHorizontalHeaderItem (0, item);

		QTimer::singleShot (20000,
				this,
				SLOT (timeredUpdateAllRequested ()));
		XmlSettingsManager::Instance ()->RegisterObject ("UpdatesCheckInterval",
				this, "handleUpdatesIntervalChanged");
	}

	Core& Core::Instance ()
	{
		static Core c;
		return c;
	}

	void Core::FinishInitialization ()
	{
		PendingManager_->Reset ();
		PopulatePluginsModel ();
	}

	void Core::Release ()
	{
		delete RepoInfoFetcher_;
		RepoInfoFetcher_ = 0;

		delete Storage_;
		Storage_ = 0;
	}

	void Core::SecondInit ()
	{
		ReadSettings ();

		UpdatesNotificationManager_ = new UpdatesNotificationManager (PackagesModel_, Proxy_, this);
		connect (UpdatesNotificationManager_,
				SIGNAL (openLackmanRequested ()),
				this,
				SIGNAL (openLackmanRequested ()));
	}

	void Core::SetProxy (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;

		RepoInfoFetcher_ = new RepoInfoFetcher (proxy, this);
		connect (RepoInfoFetcher_,
				SIGNAL (infoFetched (const RepoInfo&)),
				this,
				SLOT (handleInfoFetched (const RepoInfo&)));
		connect (RepoInfoFetcher_,
				SIGNAL (componentFetched (const PackageShortInfoList&,
						const QString&, int)),
				this,
				SLOT (handleComponentFetched (const PackageShortInfoList&,
						const QString&, int)));
		connect (RepoInfoFetcher_,
				SIGNAL (packageFetched (const PackageInfo&, int)),
				this,
				SLOT (handlePackageFetched (const PackageInfo&, int)));
	}

	ICoreProxy_ptr Core::GetProxy () const
	{
		return Proxy_;
	}

	QAbstractItemModel* Core::GetPluginsModel () const
	{
		return PackagesModel_;
	}

	PendingManager* Core::GetPendingManager () const
	{
		return PendingManager_;
	}

	ExternalResourceManager* Core::GetExtResourceManager () const
	{
		return ExternalResourceManager_;
	}

	Storage* Core::GetStorage () const
	{
		return Storage_;
	}

	QAbstractItemModel* Core::GetRepositoryModel () const
	{
		return ReposModel_;
	}

	UpdatesNotificationManager* Core::GetUpdatesNotificationManager () const
	{
		return UpdatesNotificationManager_;
	}

	DependencyList Core::GetDependencies (int packageId) const
	{
		return Util::Filter (Storage_->GetDependencies (packageId),
				[] (const auto& dep) { return dep.Type_ == Dependency::TRequires; });
	}

	QList<ListPackageInfo> Core::GetDependencyFulfillers (const Dependency& dep) const
	{
		return Storage_->GetFulfillers (dep);
	}

	bool Core::IsVersionOk (const QString& candidate, QString refVer) const
	{
		Dependency::Relation relation;

		if (refVer.startsWith (">="))
		{
			relation = Dependency::GE;
			refVer = refVer.mid (2);
		}
		else if (refVer.startsWith ("<="))
		{
			relation = Dependency::LE;
			refVer = refVer.mid (2);
		}
		else if (refVer.startsWith ('>'))
		{
			relation = Dependency::G;
			refVer = refVer.mid (1);
		}
		else if (refVer.startsWith ('<'))
		{
			relation = Dependency::L;
			refVer = refVer.mid (1);
		}
		else
		{
			relation = Dependency::E;
			if (refVer.startsWith ('='))
				refVer = refVer.mid (1);
		}
		refVer = refVer.trimmed ();

		return Relation2comparator [relation] (candidate, refVer);
	}

	bool Core::IsFulfilled (const Dependency& dep) const
	{
		const auto& all = GetAllInstalledPackages ();
		return std::any_of (all.begin (), all.end (),
				[this, &dep] (const InstalledDependencyInfo& info)
					{ return info.Dep_.Name_ == dep.Name_ && IsVersionOk (info.Dep_.Version_, dep.Version_); });
	}

	QIcon Core::GetIconForLPI (const ListPackageInfo& packageInfo)
	{
		const auto mgr = Proxy_->GetIconThemeManager ();
		QIcon result;
		switch (packageInfo.Type_)
		{
		case PackageInfo::TPlugin:
			result = mgr->GetIcon ("preferences-plugin");
			break;
		case PackageInfo::TIconset:
			result = mgr->GetIcon ("preferences-desktop-icons");
			break;
		case PackageInfo::TTranslation:
			result = mgr->GetIcon ("preferences-desktop-locale");
			break;
		case PackageInfo::TData:
		case PackageInfo::TQuark:
			result = mgr->GetIcon ("package-x-generic");
			break;
		case PackageInfo::TTheme:
			result = mgr->GetIcon ("preferences-desktop-theme");
			break;
		}
		return result;
	}

	ListPackageInfo Core::GetListPackageInfo (int packageId)
	{
		return Storage_->GetSingleListPackageInfo (packageId);
	}

	QList<QUrl> Core::GetPackageURLs (int packageId) const
	{
		QList<QUrl> result;

		const auto& repo2cmpt = Storage_->GetPackageLocations (packageId);

		PackageShortInfo info;
		try
		{
			info = Storage_->GetPackage (packageId);
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
					<< "error getting package"
					<< packageId
					<< e.what ();
			return result;
		}

		auto pathAddition = QString ("dists/%1/all/");
		const auto& normalized = LackManUtil::NormalizePackageName (info.Name_);
		const auto& version = info.Versions_.at (0);
		pathAddition += QString ("%1/%1-%2.tar.%3")
				.arg (normalized)
				.arg (version)
				.arg (info.VersionArchivers_.value (version, "gz"));

		for (auto [repoId, components] : Util::Stlize (repo2cmpt))
		{
			RepoInfo ri = Storage_->GetRepo (repoId);
			QUrl url = ri.GetUrl ();
			QString path = url.path ();
			if (!path.endsWith ('/'))
				path += '/';

			for (const auto& component : components)
			{
				QUrl tmp = url;
				tmp.setPath (path + pathAddition.arg (component));
				result << tmp;
			}
		}

		return result;
	}

	QDir Core::GetPackageDir (int packageId) const
	{
		ListPackageInfo info = Storage_->GetSingleListPackageInfo (packageId);
		QDir dir = QDir::home ();
		dir.cd (".leechcraft");

		auto SafeCD = [&dir] (const QString& subdir)
		{
			if (!dir.exists (subdir))
				dir.mkdir (subdir);
			if (!dir.cd (subdir))
				throw std::runtime_error (std::string ("Unable to cd into ") + subdir.toUtf8 ().constData ());
		};

		switch (info.Type_)
		{
		case PackageInfo::TPlugin:
			SafeCD ("plugins");
			SafeCD ("scriptable");
			SafeCD (info.Language_);
			break;
		case PackageInfo::TIconset:
			SafeCD ("icons");
			break;
		case PackageInfo::TTranslation:
			SafeCD ("translations");
			break;
		case PackageInfo::TData:
		case PackageInfo::TTheme:
		case PackageInfo::TQuark:
			SafeCD ("data");
			break;
		}
		return dir;
	}

	void Core::AddRepo (const QUrl& url)
	{
		QStandardItem *item = new QStandardItem (url.toString ());
		item->setData (url);
		ReposModel_->appendRow (item);
		RepoInfoFetcher_->FetchFor (url);
	}

	void Core::UpdateRepo (const QUrl& url, const QStringList& components)
	{
		int id = -1;
		QStringList ourComponents;
		try
		{
			id = Storage_->FindRepo (url);
			if (id == -1)
			{
				QString str;
				QDebug debug (&str);
				debug << "unable to find repo with URL"
						<< url.toString ();
				qWarning () << Q_FUNC_INFO
						<< str;
				SendEntity (Util::MakeNotification (tr ("Error updating repository"),
						tr ("Unable to find repository with URL %1.")
							.arg (url.toString ()),
						Priority::Critical));
				return;
			}
			ourComponents = Storage_->GetComponents (id);
		}
		catch (const std::exception& e)
		{
			QString str;
			QDebug debug (&str);
			debug << "unable to get ID for repo"
					<< url.toString ()
					<< "with error"
					<< e.what ();
			qWarning () << Q_FUNC_INFO
					<< str;
			SendEntity (Util::MakeNotification (tr ("Error updating repository"),
					tr ("While trying to update the repository: %1.")
						.arg (str),
					Priority::Critical));
			return;
		}

		for (const auto& oc : ourComponents)
		{
			if (!components.contains (oc))
			{
				qDebug () << Q_FUNC_INFO
						<< "orphaned component"
						<< oc;
				try
				{
					Storage_->RemoveComponent (id, oc);
				}
				catch (const std::exception& e)
				{
					qWarning () << Q_FUNC_INFO
							<< "unable to remove component"
							<< oc
							<< "not present in freshly obtained description of"
							<< id
							<< url
							<< "because of"
							<< e.what ();
					SendEntity (Util::MakeNotification (tr ("Error updating repository"),
							tr ("Unable to remove the component `%1` which "
								"disappeared from the list of components for repo %2.")
								.arg (oc)
								.arg (url.toString ()),
							Priority::Critical));
					return;
				}
			}
		}

		for (const QString& component : components)
		{
			QUrl compUrl = url;
			compUrl.setPath ((compUrl.path () + "/dists/%1/all/").arg (component));
			RepoInfoFetcher_->FetchComponent (compUrl, id, component);
		}
	}

	void Core::cancelPending ()
	{
		PendingManager_->Reset ();
	}

	void Core::acceptPending ()
	{
		const auto& toInstall = PendingManager_->GetPendingInstall ();
		const auto& toRemove = PendingManager_->GetPendingRemove ();
		const auto& toUpdate = PendingManager_->GetPendingUpdate ();

		for (int packageId : toRemove)
			PerformRemoval (packageId);

		for (int packageId : toInstall)
		{
			try
			{
				PackageProcessor_->Install (packageId);
			}
			catch (const std::exception& e)
			{
				auto str = QString::fromUtf8 (e.what ());
				qWarning () << Q_FUNC_INFO
						<< "got"
						<< str
						<< "while installing"
						<< packageId;
				SendEntity (Util::MakeNotification (tr ("Unable to install package"),
							str,
							Priority::Critical));
				continue;
			}
		}

		for (int packageId : toUpdate)
		{
			try
			{
				PackageProcessor_->Update (packageId);
			}
			catch (const std::exception& e)
			{
				auto str = QString::fromUtf8 (e.what ());
				qWarning () << Q_FUNC_INFO
						<< "got"
						<< str
						<< "while updating to"
						<< packageId;
				SendEntity (Util::MakeNotification (tr ("Unable to update package"),
							str,
							Priority::Critical));
				continue;
			}
		}
	}

	QStringList Core::GetAllTags () const
	{
		return Storage_->GetAllTags ();
	}

	InstalledDependencyInfoList Core::GetLackManInstalledPackages () const
	{
		return Storage_->GetInstalledPackages ();
	}

	InstalledDependencyInfoList Core::GetAllInstalledPackages () const
	{
		return LackManUtil::GetSystemInstalledPackages (Proxy_->GetVersion ()) +
				GetLackManInstalledPackages ();
	}

	void Core::PopulatePluginsModel ()
	{
		QMap<QString, QList<ListPackageInfo>> infos;
		try
		{
			infos = Storage_->GetListPackageInfos ();
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to get package infos"
					<< e.what ();
			return;
		}

		InstalledDependencyInfoList instedAll = GetLackManInstalledPackages ();

		for (auto list : infos)
		{
			std::sort (list.begin (), list.end (),
					[] (const ListPackageInfo& i1, const ListPackageInfo& i2)
						{ return IsVersionLess (i1.Version_, i2.Version_); });
			ListPackageInfo last = list.last ();

			for (const auto& idi : instedAll)
				if (last.Name_ == idi.Dep_.Name_)
				{
					last.IsInstalled_ = true;

					if (idi.Source_ == InstalledDependencyInfo::SLackMan &&
							IsVersionLess (idi.Dep_.Version_, last.Version_))
						last.HasNewVersion_ = true;

					break;
				}

			PackagesModel_->AddRow (last);
		}
	}

	void Core::HandleNewPackages (const PackageShortInfoList& shortInfos,
			int componentId, const QString& component, const QUrl& repoUrl)
	{
		QMap<QString, QList<QString>> PackageName2NewVersions_;

		int newPackages = 0;
		for (const auto& info : shortInfos)
			for (const QString& version : info.Versions_)
			{
				int packageId = -1;
				try
				{
					packageId = Storage_->FindPackage (info.Name_, version);
				}
				catch (const std::exception& e)
				{
					qWarning () << Q_FUNC_INFO
							<< "unable to get package ID for"
							<< info.Name_
							<< version
							<< e.what ();
					SendEntity (Util::MakeNotification (tr ("Error parsing component"),
							tr ("Unable to load package ID for package `%1`-%2")
								.arg (info.Name_)
								.arg (version),
							Priority::Critical));
					return;
				}

				if (packageId == -1)
				{
					PackageName2NewVersions_ [info.Name_] << version;
					++newPackages;
				}

				try
				{
					if (packageId != -1 &&
							!Storage_->HasLocation (packageId, componentId))
						Storage_->AddLocation (packageId, componentId);
				}
				catch (const std::exception& e)
				{
					qWarning () << Q_FUNC_INFO
							<< "unable to add package location"
							<< info.Name_
							<< version
							<< e.what ();
					SendEntity (Util::MakeNotification (tr ("Error parsing component"),
							tr ("Unable to save package location for "
								"package `%1`-%2 and component %3")
								.arg (info.Name_)
								.arg (version)
								.arg (component),
							Priority::Critical));
					return;
				}
			}

		for (const QString& packageName : PackageName2NewVersions_.keys ())
		{
			auto packageUrl = repoUrl;
			const auto& normalized = LackManUtil::NormalizePackageName (packageName);
			packageUrl.setPath (packageUrl.path () +
					"/dists/" + component + "/all" +
					'/' + normalized +
					'/');
			RepoInfoFetcher_->ScheduleFetchPackageInfo (packageUrl,
					packageName,
					PackageName2NewVersions_ [packageName],
					componentId);
		}

		if (newPackages)
			SendEntity (Util::MakeNotification (tr ("Repositories updated"),
					tr ("Got %n new or updated packages, "
						"open LackMan tab to view them.",
						0, newPackages),
					Priority::Info));
	}

	void Core::PerformRemoval (int packageId)
	{
		try
		{
			PackageProcessor_->Remove (packageId);
		}
		catch (const std::exception& e)
		{
			auto str = QString::fromUtf8 (e.what ());
			qWarning () << Q_FUNC_INFO
					<< "got"
					<< str
					<< "while removing"
					<< packageId;
			SendEntity (Util::MakeNotification (tr ("Unable to remove package"),
						str,
						Priority::Critical));
			return;
		}

		if (!RecordUninstalled (packageId))
			return;

		UpdateRowFor (packageId);

		PendingManager_->SuccessfullyRemoved (packageId);

		emit packageRowActionFinished (GetPackageRow (packageId));
	}

	void Core::UpdateRowFor (int packageId)
	{
		try
		{
			const auto& info = Storage_->GetSingleListPackageInfo (packageId);
			PackagesModel_->UpdateRow (info);
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
					<< "while updating row for package"
					<< packageId
					<< "got this exception:"
					<< e.what ();
		}
	}

	bool Core::RecordInstalled (int packageId)
	{
		try
		{
			Storage_->AddToInstalled (packageId);
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
					<< "while trying to record installed package"
					<< e.what ();
			SendEntity (Util::MakeNotification (tr ("Error installing package"),
						tr ("Error recording package to the package DB."),
						Priority::Critical));

			try
			{
				PackageProcessor_->Remove (packageId);
			}
			catch (const std::exception& e)
			{
				qWarning () << Q_FUNC_INFO
						<< "while trying to cleanup partially installed package"
						<< e.what ();
			}
			return false;
		}

		return true;
	}

	bool Core::RecordUninstalled (int packageId)
	{
		try
		{
			Storage_->RemoveFromInstalled (packageId);
		}
		catch (const std::exception& e)
		{
			auto str = QString::fromUtf8 (e.what ());
			qWarning () << Q_FUNC_INFO
					<< "unable to remove from installed"
					<< packageId
					<< str;
			SendEntity (Util::MakeNotification (tr ("Unable to remove package"),
						str,
						Priority::Critical));
			return false;
		}

		return true;
	}

	int Core::GetPackageRow (int packageId) const
	{
		return PackagesModel_->GetRow (packageId);
	}

	void Core::ReadSettings ()
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_LackMan");
		int size = settings.beginReadArray ("Repos");
		for (int i = 0; i < size; ++i)
		{
			settings.setArrayIndex (i);
			QUrl url = settings.value ("URL").value<QUrl> ();
			AddRepo (url);
		}
		settings.endArray ();

		if (settings.value ("AddDefault", true).toBool ())
		{
			AddRepo (QUrl ("https://leechcraft.org/repo/"));
			settings.setValue ("AddDefault", false);
			WriteSettings ();
		}
	}

	void Core::WriteSettings ()
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_LackMan");
		settings.beginWriteArray ("Repos");
		for (int i = 0, size = ReposModel_->rowCount ();
				i < size; ++i)
		{
			settings.setArrayIndex (i);
			QStandardItem *item = ReposModel_->item (i);
			QUrl url = item->data ().value<QUrl> ();
			settings.setValue ("URL", url);
		}
		settings.endArray ();
	}

	void Core::updateAllRequested ()
	{
		for (int i = 0, size = ReposModel_->rowCount ();
				i < size; ++i)
		{
			QStandardItem *item = ReposModel_->item (i);
			QUrl url = item->data ().value<QUrl> ();
			QStringList components;

			try
			{
				int id = Storage_->FindRepo (url);
				components = Storage_->GetRepo (id).GetComponents ();
			}
			catch (const std::exception& e)
			{
				qWarning () << Q_FUNC_INFO
						<< "while trying to get repo components for"
						<< url
						<< e.what ();
				continue;
			}
			catch (...)
			{
				qWarning () << Q_FUNC_INFO
						<< "general error while trying to get repo components for"
						<< url;
				continue;
			}

			qDebug () << "would update" << url << components;
			UpdateRepo (url, components);
		}
	}

	void Core::handleUpdatesIntervalChanged ()
	{
		const int hours = XmlSettingsManager::Instance ()->
				property ("UpdatesCheckInterval").toInt ();
		if (hours && !UpdatesEnabled_)
			timeredUpdateAllRequested ();
		UpdatesEnabled_ = hours;
	}

	void Core::timeredUpdateAllRequested ()
	{
		updateAllRequested ();

		const int hours = XmlSettingsManager::Instance ()->
				property ("UpdatesCheckInterval").toInt ();
		if (hours)
			QTimer::singleShot (hours * 3600 * 1000,
					this,
					SLOT (timeredUpdateAllRequested ()));
		else
			UpdatesEnabled_ = false;
	}

	void Core::upgradeAllRequested ()
	{
		for (int i = 0, rows = PackagesModel_->rowCount ();
				i < rows; ++i)
		{
			const auto& index = PackagesModel_->index (i, 0);
			int packageId = PackagesModel_->data (index, PackagesModel::PMRPackageID).toInt ();
			bool isUpgr = PackagesModel_->data (index, PackagesModel::PMRUpgradable).toBool ();

			if (isUpgr)
				PendingManager_->ToggleUpdate (packageId, true);
		}
	}

	void Core::removeRequested (const QString&, const QModelIndexList& list)
	{
		auto rows = Util::Map (list, &QModelIndex::row);

		std::sort (rows.begin (), rows.end ());
		std::reverse (rows.begin (), rows.end ());

		for (auto row : rows)
		{
			QList<QStandardItem*> items = ReposModel_->takeRow (row);
			QUrl url = items.at (RCURL)->data ().value<QUrl> ();

			try
			{
				int repoId = Storage_->FindRepo (url);
				Storage_->RemoveRepo (repoId);
			}
			catch (const std::exception& e)
			{
				qWarning () << Q_FUNC_INFO
						<< "unable to remove repo"
						<< url
						<< e.what ();
			}

			qDeleteAll (items);
		}

		WriteSettings ();
	}

	void Core::addRequested (const QString&, const QVariantList& list)
	{
		if (!list.size ())
		{
			qWarning () << Q_FUNC_INFO
					<< "too small list";
			return;
		}

		QString str = list.at (0).toString ();
		QUrl url = QUrl (str);
		if (!url.isValid ())
		{
			qWarning () << Q_FUNC_INFO
					<< "incorrect url"
					<< str;
			SendEntity (Util::MakeNotification (tr ("Repository addition error"),
					tr ("Incorrect URL %1.")
						.arg (str),
					Priority::Critical));
			return;
		}

		AddRepo (url);

		WriteSettings ();
	}

	void Core::handleInfoFetched (const RepoInfo& ri)
	{
		int repoId = -1;
		try
		{
			repoId = Storage_->FindRepo (ri.GetUrl ());
			if (repoId == -1)
				repoId = Storage_->AddRepo (ri);
		}
		catch (const std::exception& e)
		{
			QString str;
			QDebug debug (&str);
			debug << "unable to find/add repo"
					<< ri.GetName ()
					<< ri.GetUrl ()
					<< "with error"
					<< e.what ();
			qWarning () << Q_FUNC_INFO
					<< str;
			SendEntity (Util::MakeNotification (tr ("Error adding/updating repository"),
					tr ("While trying to add or update the repository: %1.")
						.arg (str),
					Priority::Critical));
		}

		if (repoId == -1)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to add repo"
					<< ri.GetUrl ()
					<< ri.GetName ();
			return;
		}

		UpdateRepo (ri.GetUrl (), ri.GetComponents ());
	}

	void Core::handleComponentFetched (const PackageShortInfoList& shortInfos,
			const QString& component, int repoId)
	{
		int componentId = -1;
		QUrl repoUrl;
		try
		{
			componentId = Storage_->FindComponent (repoId, component);
			if (componentId == -1)
				componentId = Storage_->AddComponent (repoId, component);

			repoUrl = Storage_->GetRepo (repoId).GetUrl ();
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to get component ID for"
					<< component
					<< "of"
					<< repoId
					<< e.what ();
			SendEntity (Util::MakeNotification (tr ("Error parsing component"),
					tr ("Unable to load component ID for component %1.")
						.arg (component),
					Priority::Critical));
			return;
		}

		QList<int> presentPackages;
		QSet<int> installedPackages;
		try
		{
			presentPackages = Storage_->GetPackagesInComponent (componentId);
			installedPackages = Storage_->GetInstalledPackagesIDs ();
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to get installed or present packages in component:"
					<< e.what ();
			SendEntity (Util::MakeNotification (tr ("Error handling component"),
					tr ("Unable to load packages already present in the component %1.")
						.arg (component),
					Priority::Critical));
			return;
		}

		for (int presentPId : presentPackages)
		{
			PackageShortInfo psi;
			try
			{
				psi = Storage_->GetPackage (presentPId);
			}
			catch (const std::exception& e)
			{
				qWarning () << Q_FUNC_INFO
						<< "unable to get present package:"
						<< e.what ();
				SendEntity (Util::MakeNotification (tr ("Error handling component"),
						tr ("Unable to load package already present in the component %1.")
							.arg (component),
						Priority::Critical));
				return;
			}

			const QString& ourVersion = psi.Versions_.at (0);
			bool found = false;
			for (const auto& candidate : shortInfos)
			{
				if (candidate.Name_ == psi.Name_ &&
						candidate.Versions_.contains (ourVersion))
				{
					found = true;
					break;
				}
			}

			if (!found)
			{
				try
				{
					Storage_->RemoveLocation (presentPId, componentId);

					if (!installedPackages.contains (presentPId))
						Storage_->RemovePackage (presentPId);
				}
				catch (const std::exception& e)
				{
					qWarning () << Q_FUNC_INFO
							<< "unable to remove package for:"
							<< component
							<< presentPId
							<< e.what ();
					SendEntity (Util::MakeNotification (tr ("Error handling component"),
							tr ("Unable to remove package which has been removed upstream from %1.")
								.arg (component),
							Priority::Critical));
					return;
				}
			}
		}

		HandleNewPackages (shortInfos, componentId, component, repoUrl);
	}

	void Core::handlePackageFetched (const PackageInfo& pInfo,
			int componentId)
	{
		try
		{
			Storage_->AddPackages (pInfo);

			QStringList versions = pInfo.Versions_;
			std::sort (versions.begin (), versions.end (), IsVersionLess);
			const auto& greatest = versions.last ();

			for (const auto& version : pInfo.Versions_)
			{
				const int packageId = Storage_->FindPackage (pInfo.Name_, version);
				Storage_->AddLocation (packageId, componentId);

				if (version == greatest)
				{
					const auto& existing = PackagesModel_->FindPackage (pInfo.Name_).Version_;
					if (existing.isEmpty ())
						PackagesModel_->AddRow (Storage_->GetSingleListPackageInfo (packageId));
					else if (IsVersionLess (existing, greatest))
					{
						auto info = Storage_->GetSingleListPackageInfo (packageId);
						info.HasNewVersion_ = info.IsInstalled_;
						PackagesModel_->UpdateRow (info);
					}
				}
			}

			emit tagsUpdated (GetAllTags ());
		}
		catch (const std::runtime_error& e)
		{
			pInfo.Dump ();
			qWarning () << Q_FUNC_INFO
					<< e.what ();
			SendEntity (Util::MakeNotification (tr ("Error retrieving package"),
					tr ("Unable to save package %1.")
						.arg (pInfo.Name_),
					Priority::Critical));
		}

		if (pInfo.IconURL_.isValid ())
		{
			try
			{
				ExternalResourceManager_->GetResourceData (pInfo.IconURL_);
			}
			catch (const std::runtime_error& e)
			{
				qWarning () << Q_FUNC_INFO
						<< "error fetching icon from"
						<< pInfo.IconURL_
						<< e.what ();
				SendEntity (Util::MakeNotification (tr ("Error retrieving package icon"),
						tr ("Unable to retrieve icon for package %1.")
							.arg (pInfo.Name_),
						Priority::Critical));
			}
		}
	}

	void Core::handlePackageInstallError (int packageId, const QString& error)
	{
		QString packageName;
		try
		{
			packageName = Storage_->GetPackage (packageId).Name_;
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
					<< "while trying to get erroneous package name"
					<< e.what ();
		}

		QString prepared = tr ("Error while fetching package %1: %2.");
		QString msg;
		if (packageName.size ())
			msg = prepared.arg (packageName).arg (error);
		else
			msg = prepared.arg (packageId).arg (error);

		SendEntity (Util::MakeNotification (tr ("Error installing package"),
					msg,
					Priority::Critical));
	}

	void Core::handlePackageInstalled (int packageId)
	{
		if (!RecordInstalled (packageId))
			return;

		UpdateRowFor (packageId);

		PendingManager_->SuccessfullyInstalled (packageId);

		QString packageName;
		try
		{
			packageName = Storage_->GetPackage (packageId).Name_;
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
					<< "while trying to get installed package name"
					<< e.what ();
			return;
		}

		SendEntity (Util::MakeNotification (tr ("Package installed"),
					tr ("Package %1 installed successfully.")
						.arg ("<em>" + packageName + "</em>"),
					Priority::Info));

		emit packageRowActionFinished (GetPackageRow (packageId));
	}

	void Core::handlePackageUpdated (int fromId, int packageId)
	{
		if (!RecordUninstalled (fromId) ||
				!RecordInstalled (packageId))
			return;

		UpdateRowFor (packageId);

		PendingManager_->SuccessfullyUpdated (packageId);

		QString packageName;
		try
		{
			packageName = Storage_->GetPackage (packageId).Name_;
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
					<< "while trying to get installed package name"
					<< e.what ();
			return;
		}

		SendEntity (Util::MakeNotification (tr ("Package updated"),
					tr ("Package %1 updated successfully.")
						.arg ("<em>" + packageName + "</em>"),
					Priority::Info));

		emit packageRowActionFinished (GetPackageRow (packageId));
	}

	void Core::handlePackageRemoved (int packageId)
	{
		PackagesModel_->RemovePackage (packageId);
	}
}
}
