/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "viewmanager.h"
#include <QStandardItemModel>
#include <QQmlEngine>
#include <QQmlContext>
#include <QtDebug>
#include <QDir>
#include <QSettings>
#include <QCoreApplication>
#include <QToolBar>
#include <QMainWindow>
#include <util/sll/containerconversions.h>
#include <util/sll/scopeguards.h>
#include <util/sys/paths.h>
#include <util/qml/colorthemeproxy.h>
#include <util/qml/themeimageprovider.h>
#include <util/shortcuts/shortcutmanager.h>
#include <util/util.h>
#include <util/models/rolenamesmixin.h>
#include <interfaces/iquarkcomponentprovider.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/core/irootwindowsmanager.h>
#include "sbview.h"
#include "quarkproxy.h"
#include "quarkmanager.h"
#include "viewgeometrymanager.h"
#include "viewsettingsmanager.h"
#include "viewpropsmanager.h"
#include "dirwatcher.h"

namespace LC::SB2
{
	namespace
	{
		const QString ImageProviderID = QStringLiteral ("ThemeIcons");

		class ViewItemsModel : public Util::RoleNamesMixin<QStandardItemModel>
		{
		public:
			enum Role
			{
				SourceURL = Qt::UserRole + 1,
				QuarkHasSettings,
				QuarkClass
			};

			explicit ViewItemsModel (QObject *parent)
			: RoleNamesMixin<QStandardItemModel> (parent)
			{
				QHash<int, QByteArray> names;
				names [Role::SourceURL] = QByteArrayLiteral ("sourceURL");
				names [Role::QuarkHasSettings] = QByteArrayLiteral ("quarkHasSettings");
				names [Role::QuarkClass] = QByteArrayLiteral ("quarkClass");
				setRoleNames (names);
			}
		};
	}

	ViewManager::ViewManager (Util::ShortcutManager *shortcutMgr, QMainWindow *window, QObject *parent)
	: QObject (parent)
	, ViewItemsModel_ (new ViewItemsModel (this))
	, View_ (new SBView)
	, Toolbar_ (new QToolBar (tr ("SB2 panel")))
	, Window_ (window)
	, IsDesktopMode_ (qApp->arguments ().contains (QStringLiteral ("--desktop")))
	, OnloadWindowIndex_ (GetWindowIndex ())
	, SettingsManager_ (new ViewSettingsManager (this))
	, GeomManager_ (new ViewGeometryManager (this))
	{
		const auto& file = Util::GetSysPath (Util::SysPath::QML, QStringLiteral ("sb2"), QStringLiteral ("SideView.qml"));
		if (file.isEmpty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "file not found";
			return;
		}

		new ViewPropsManager (this, SettingsManager_, this);

		for (const auto& cand : Util::GetPathCandidates (Util::SysPath::QML, {}))
			View_->engine ()->addImportPath (cand);

		View_->rootContext ()->setContextProperties ({
				{ QStringLiteral ("itemsModel"), QVariant::fromValue (ViewItemsModel_) },
				{ QStringLiteral ("quarkProxy"), QVariant::fromValue (new QuarkProxy (this, this)) },
				{ QStringLiteral ("colorProxy"),
						QVariant::fromValue (new Util::ColorThemeProxy (GetProxyHolder ()->GetColorThemeManager (), this)) },
				{ QStringLiteral ("quarkContext"), "panel_" + QString::number (GetWindowIndex ()) },

				{ QStringLiteral ("settingsModeTooltip"), tr ("Settings mode") },
				{ QStringLiteral ("quarkOrderTooltip"), tr ("Quarks order") },
				{ QStringLiteral ("addQuarkTooltip"), tr ("Add quark") },
				{ QStringLiteral ("showPanelSettingsTooltip"), tr ("Show panel settings") },
			});
		View_->engine ()->addImageProvider (ImageProviderID, new Util::ThemeImageProvider (GetProxyHolder ()));

		Toolbar_->addWidget (View_);
		View_->setVisible (true);

		GeomManager_->Manage ();

		View_->setSource (QUrl::fromLocalFile (file));

		LoadRemovedList ();
		LoadQuarkOrder ();

		auto toggleAct = Toolbar_->toggleViewAction ();
		toggleAct->setProperty ("ActionIcon", "layer-visible-on");
		toggleAct->setShortcut (QStringLiteral ("Ctrl+J,S"));
		shortcutMgr->RegisterAction ("TogglePanel", toggleAct);

		window->addAction (toggleAct);
	}

	SBView* ViewManager::GetView () const
	{
		return View_;
	}

	QToolBar* ViewManager::GetToolbar () const
	{
		return Toolbar_;
	}

	QMainWindow* ViewManager::GetManagedWindow () const
	{
		return Window_;
	}

	QRect ViewManager::GetFreeCoords () const
	{
		QRect result = Window_->rect ();
		result.moveTopLeft (Window_->mapToGlobal (QPoint { 0, 0 }));

		if (!IsDesktopMode_)
			switch (Window_->toolBarArea (Toolbar_))
			{
			case Qt::LeftToolBarArea:
				result.setLeft (result.left () + Toolbar_->width ());
				break;
			case Qt::RightToolBarArea:
				result.setRight (result.right () - Toolbar_->width ());
				break;
			case Qt::TopToolBarArea:
				result.setTop (result.top () + Toolbar_->height ());
				break;
			case Qt::BottomToolBarArea:
				result.setBottom (result.bottom () - Toolbar_->height ());
				break;
			default:
				break;
			}

		return result;
	}

	ViewSettingsManager* ViewManager::GetViewSettingsManager () const
	{
		return SettingsManager_;
	}

	bool ViewManager::IsDesktopMode () const
	{
		return IsDesktopMode_;
	}

	void ViewManager::SecondInit ()
	{
		for (const auto& component : FindAllQuarks ())
			AddComponent (component, false);

		auto watcher = new DirWatcher (Util::CreateIfNotExists (QStringLiteral ("data/quarks")), this);
		connect (watcher,
				&DirWatcher::quarksAdded,
				this,
				&ViewManager::HandleQuarksAdded);
		connect (watcher,
				&DirWatcher::quarksRemoved,
				this,
				&ViewManager::HandleQuarksRemoved);

		SaveQuarkOrder ();
	}

	void ViewManager::RegisterInternalComponent (const QuarkComponent_ptr& c)
	{
		InternalComponents_ << c;
	}

	void ViewManager::RemoveQuark (const QUrl& url)
	{
		RemoveQuarkBy<ViewItemsModel::Role::SourceURL> (url);
	}

	void ViewManager::RemoveQuark (const QString& id)
	{
		RemoveQuarkBy<ViewItemsModel::Role::QuarkClass> (id);
	}

	void ViewManager::SetupContext (const QUrl& url, QQmlContext *context)
	{
		const auto manager = Quark2Manager_ [url];
		if (!manager)
		{
			qWarning () << Q_FUNC_INFO
					<< "no manager for"
					<< url;
			return;
		}
		manager->SetupContext (context);
	}

	template<int Role, typename T>
	void ViewManager::RemoveQuarkBy (const T& val)
	{
		for (int i = 0; i < ViewItemsModel_->rowCount (); ++i)
		{
			auto item = ViewItemsModel_->item (i);
			if (item->data (Role) != val)
				continue;

			auto url = item->data (ViewItemsModel::Role::SourceURL).toUrl ();
			auto mgr = Quark2Manager_.take (url);
			if (!mgr)
			{
				qWarning () << Q_FUNC_INFO
						<< "no manager for"
						<< url;
				return;
			}

			ViewItemsModel_->removeRow (i);
			AddToRemoved (mgr->GetManifest ().GetID ());

			SaveQuarkOrder ();

			break;
		}
	}

	void ViewManager::UnhideQuark (const QuarkComponent_ptr& component, const QuarkManager_ptr& manager)
	{
		if (!manager)
			return;

		RemoveFromRemoved (manager->GetManifest ().GetID ());

		AddComponent (component, manager, true);

		SaveQuarkOrder ();
	}

	void ViewManager::MoveQuark (int from, int to)
	{
		if (from < to)
			--to;
		ViewItemsModel_->insertRow (to, ViewItemsModel_->takeRow (from));

		SaveQuarkOrder ();
	}

	void ViewManager::MovePanel (Qt::ToolBarArea area)
	{
		GeomManager_->SetPosition (area);
	}

	namespace
	{
		QuarkComponents_t ScanRootDir (const QDir& dir)
		{
			QuarkComponents_t result;
			for (const auto& entry : dir.entryList (QDir::Dirs | QDir::NoDotAndDotDot | QDir::Readable))
			{
				QDir quarkDir (dir);
				quarkDir.cd (entry);
				if (!quarkDir.exists (entry + ".qml"))
					continue;

				auto c = std::make_shared<QuarkComponent> ();
				c->Url_ = QUrl::fromLocalFile (quarkDir.absoluteFilePath (entry + ".qml"));
				result << c;
			}
			return result;
		}
	}

	QuarkComponents_t ViewManager::FindAllQuarks () const
	{
		auto result = InternalComponents_;

		for (const auto& cand : Util::GetPathCandidates (Util::SysPath::QML, QStringLiteral ("quarks")))
			result += ScanRootDir (QDir (cand));

		const auto& local = Util::CreateIfNotExists (QStringLiteral ("data/quarks"));
		result += ScanRootDir (local);

		auto pm = GetProxyHolder ()->GetPluginsManager ();
		for (auto prov : pm->GetAllCastableTo<IQuarkComponentProvider*> ())
			result += prov->GetComponents ();

		return result;
	}

	QList<QUrl> ViewManager::GetAddedQuarks () const
	{
		QList<QUrl> result;

		for (int i = 0, rc = ViewItemsModel_->rowCount (); i < rc; ++i)
		{
			const auto item = ViewItemsModel_->item (i);
			result << item->data (ViewItemsModel::Role::SourceURL).toUrl ();
		}

		return result;
	}

	QuarkManager_ptr ViewManager::GetAddedQuarkManager (const QUrl& url) const
	{
		return Quark2Manager_ [url];
	}

	std::shared_ptr<QSettings> ViewManager::GetSettings () const
	{
		const auto subSet = GetWindowIndex () || IsDesktopMode_;

		const auto& org = QCoreApplication::organizationName ();
		const auto& app = QCoreApplication::applicationName () + "_SB2";
		std::shared_ptr<QSettings> result (new QSettings (org, app),
				[subSet] (QSettings *settings)
				{
					if (subSet)
						settings->endGroup ();
					delete settings;
				});

		if (subSet)
			result->beginGroup (QStringLiteral ("%1_%2")
					.arg (OnloadWindowIndex_)
					.arg (IsDesktopMode_));

		return result;
	}

	void ViewManager::AddComponent (const QuarkComponent_ptr& comp, bool force)
	{
		try
		{
			AddComponent (comp, std::make_shared<QuarkManager> (comp, this), force);
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
					<< e.what ();
			return;
		}
	}

	void ViewManager::AddComponent (const QuarkComponent_ptr& comp, const QuarkManager_ptr& mgr, bool force)
	{
		if (!mgr->IsValidArea ())
			return;

		const auto& quarkId = mgr->GetManifest ().GetID ();

		if (!force)
		{
			if (mgr->GetManifest ().IsHiddenByDefault () &&
					!PreviousQuarkOrder_.contains (quarkId))
				return;

			if (RemovedIDs_.contains (quarkId))
				return;
		}

		Quark2Manager_ [comp->Url_] = mgr;

		auto item = new QStandardItem;
		item->setData (comp->Url_, ViewItemsModel::Role::SourceURL);
		item->setData (mgr->HasSettings (), ViewItemsModel::Role::QuarkHasSettings);
		item->setData (quarkId, ViewItemsModel::Role::QuarkClass);

		const int pos = PreviousQuarkOrder_.indexOf (quarkId);
		if (pos == -1 || pos == PreviousQuarkOrder_.size () - 1)
			ViewItemsModel_->appendRow (item);
		else
		{
			bool added = false;
			for (int i = pos + 1; i < PreviousQuarkOrder_.size (); ++i)
			{
				const auto& thatId = PreviousQuarkOrder_.at (i);
				for (int j = 0; j < ViewItemsModel_->rowCount (); ++j)
				{
					if (ViewItemsModel_->item (j)->data (ViewItemsModel::Role::QuarkClass) != thatId)
						continue;

					ViewItemsModel_->insertRow (j, item);
					added = true;
					break;
				}

				if (added)
					break;
			}
			if (!added)
				ViewItemsModel_->appendRow (item);
		}
	}

	void ViewManager::AddToRemoved (const QString& id)
	{
		RemovedIDs_ << id;
		SaveRemovedList ();
	}

	void ViewManager::RemoveFromRemoved (const QString& id)
	{
		RemovedIDs_.remove (id);
		SaveRemovedList ();
	}

	void ViewManager::SaveRemovedList () const
	{
		auto settings = GetSettings ();
		auto groupGuard = Util::BeginGroup (*settings, QStringLiteral ("RemovedList"));
		settings->setValue (QStringLiteral ("IDs"), QStringList (RemovedIDs_.values ()));
	}

	void ViewManager::LoadRemovedList ()
	{
		auto settings = GetSettings ();
		auto groupGuard = Util::BeginGroup (*settings, QStringLiteral ("RemovedList"));
		RemovedIDs_ = Util::AsSet (settings->value (QStringLiteral ("IDs")).toStringList ());
	}

	void ViewManager::SaveQuarkOrder ()
	{
		PreviousQuarkOrder_.clear ();
		for (int i = 0; i < ViewItemsModel_->rowCount (); ++i)
		{
			auto item = ViewItemsModel_->item (i);
			PreviousQuarkOrder_ << item->data (ViewItemsModel::Role::QuarkClass).toString ();
		}

		auto settings = GetSettings ();
		auto groupGuard = Util::BeginGroup (*settings, QStringLiteral ("QuarkOrder"));
		settings->setValue (QStringLiteral ("IDs"), PreviousQuarkOrder_);
	}

	void ViewManager::LoadQuarkOrder ()
	{
		auto settings = GetSettings ();
		auto groupGuard = Util::BeginGroup (*settings, QStringLiteral ("QuarkOrder"));
		PreviousQuarkOrder_ = settings->value (QStringLiteral ("IDs")).toStringList ();
	}

	int ViewManager::GetWindowIndex () const
	{
		auto rootWM = GetProxyHolder ()->GetRootWindowsManager ();
		return rootWM->GetWindowIndex (Window_);
	}

	void ViewManager::HandleQuarksAdded (const QList<QUrl>& urls)
	{
		qDebug () << Q_FUNC_INFO << urls;
		for (const auto& url : urls)
		{
			auto c = std::make_shared<QuarkComponent> ();
			c->Url_ = url;
			AddComponent (c, false);
		}
	}

	void ViewManager::HandleQuarksRemoved (const QList<QUrl>& urls)
	{
		qDebug () << Q_FUNC_INFO << urls;
		for (const auto& url : urls)
		{
			const auto& id = Quark2Manager_ [url]->GetManifest ().GetID ();
			RemoveQuark (url);
			RemoveFromRemoved (id);
		}
	}
}
