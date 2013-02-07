/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "viewmanager.h"
#include <QStandardItemModel>
#include <QDeclarativeEngine>
#include <QDeclarativeContext>
#include <QtDebug>
#include <QDir>
#include <QSettings>
#include <QCoreApplication>
#include <QToolBar>
#include <QMainWindow>
#include <util/sys/paths.h>
#include <util/qml/colorthemeproxy.h>
#include <util/qml/themeimageprovider.h>
#include <interfaces/iquarkcomponentprovider.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/core/irootwindowsmanager.h>
#include "sbview.h"
#include "quarkproxy.h"
#include "quarkmanager.h"

namespace LeechCraft
{
namespace SB2
{
	namespace
	{
		const QString ImageProviderID = "ThemeIcons";

		class ViewItemsModel : public QStandardItemModel
		{
		public:
			enum Role
			{
				SourceURL= Qt::UserRole + 1,
				QuarkHasSettings,
				QuarkClass
			};

			ViewItemsModel (QObject *parent)
			: QStandardItemModel (parent)
			{
				QHash<int, QByteArray> names;
				names [Role::SourceURL] = "sourceURL";
				names [Role::QuarkHasSettings] = "quarkHasSettings";
				names [Role::QuarkClass] = "quarkClass";
				setRoleNames (names);
			}
		};
	}

	ViewManager::ViewManager (ICoreProxy_ptr proxy, QMainWindow *window, QObject *parent)
	: QObject (parent)
	, Proxy_ (proxy)
	, ViewItemsModel_ (new ViewItemsModel (this))
	, View_ (new SBView)
	, Toolbar_ (new QToolBar (tr ("SB2 panel")))
	, Window_ (window)
	{
		const auto& file = Util::GetSysPath (Util::SysPath::QML, "sb2", "SideView.qml");
		if (file.isEmpty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "file not found";
			return;
		}

		for (const auto& cand : Util::GetPathCandidates (Util::SysPath::QML, ""))
			View_->engine ()->addImportPath (cand);

		View_->rootContext ()->setContextProperty ("itemsModel", ViewItemsModel_);
		View_->rootContext ()->setContextProperty ("quarkProxy", new QuarkProxy (this, proxy, this));
		View_->rootContext ()->setContextProperty ("colorProxy",
				new Util::ColorThemeProxy (proxy->GetColorThemeManager (), this));
		View_->engine ()->addImageProvider (ImageProviderID, new Util::ThemeImageProvider (proxy));

		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_SB2");
		settings.beginGroup ("Toolbars");
		const auto& posSettingName = "Pos_" + QString::number (GetWindowIndex ());
		auto pos = settings.value (posSettingName, static_cast<int> (Qt::LeftToolBarArea)).toInt ();
		settings.endGroup ();

		setOrientation ((pos == Qt::LeftToolBarArea || pos == Qt::RightToolBarArea) ? Qt::Vertical : Qt::Horizontal);

		View_->setSource (QUrl::fromLocalFile (file));

		LoadRemovedList ();
		LoadQuarkOrder ();

		Toolbar_->addWidget (View_);
		Toolbar_->setFloatable (false);
		View_->setVisible (true);
		connect (Toolbar_,
				SIGNAL (orientationChanged (Qt::Orientation)),
				this,
				SLOT (setOrientation (Qt::Orientation)));
		connect (Toolbar_,
				SIGNAL (topLevelChanged (bool)),
				this,
				SLOT (handleToolbarTopLevel (bool)));

		window->addToolBar (static_cast<Qt::ToolBarArea> (pos), Toolbar_);
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

	void ViewManager::SecondInit ()
	{
		for (const auto& component : FindAllQuarks ())
			AddComponent (component);
	}

	void ViewManager::RegisterInternalComponent (const QuarkComponent& c)
	{
		InternalComponents_ << c;
	}

	void ViewManager::ShowSettings (const QUrl& url)
	{
		auto manager = Quark2Manager_ [url];
		manager->ShowSettings ();
	}

	void ViewManager::RemoveQuark (const QUrl& url)
	{
		for (int i = 0; i < ViewItemsModel_->rowCount (); ++i)
		{
			auto item = ViewItemsModel_->item (i);
			if (item->data (ViewItemsModel::Role::SourceURL) != url)
				continue;

			ViewItemsModel_->removeRow (i);
		}

		auto mgr = Quark2Manager_.take (url);
		AddToRemoved (mgr->GetID ());
	}

	void ViewManager::RemoveQuark (const QString& id)
	{
		QUrl url;
		for (int i = 0; i < ViewItemsModel_->rowCount (); ++i)
		{
			auto item = ViewItemsModel_->item (i);
			if (item->data (ViewItemsModel::Role::QuarkClass) != id)
				continue;

			url = item->data (ViewItemsModel::Role::SourceURL).toUrl ();
			ViewItemsModel_->removeRow (i);
		}

		if (!url.isValid ())
			return;

		auto mgr = Quark2Manager_.take (url);
		AddToRemoved (mgr->GetID ());
	}

	void ViewManager::UnhideQuark (const QuarkComponent& component, QuarkManager_ptr manager)
	{
		if (!manager)
			return;

		RemoveFromRemoved (manager->GetID ());

		AddComponent (component, manager);
	}

	void ViewManager::MoveQuark (int from, int to)
	{
		if (from < to)
			--to;
		ViewItemsModel_->insertRow (to, ViewItemsModel_->takeRow (from));

		SaveQuarkOrder ();
	}

	QList<QuarkComponent> ViewManager::FindAllQuarks () const
	{
		auto result = InternalComponents_;

		for (const auto& cand : Util::GetPathCandidates (Util::SysPath::QML, "quarks"))
			result += ScanRootDir (QDir (cand));

		QDir local = QDir::home ();
		if (local.cd (".leechcraft") &&
			local.cd ("data") &&
			local.cd ("quarks"))
			result += ScanRootDir (local);

		auto pm = Proxy_->GetPluginsManager ();
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

	void ViewManager::AddComponent (const QuarkComponent& comp)
	{
		QuarkManager_ptr mgr;
		try
		{
			mgr.reset (new QuarkManager (comp, this, Proxy_));
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
					<< e.what ();
			return;
		}

		AddComponent (comp, mgr);
	}

	void ViewManager::AddComponent (const QuarkComponent& comp, QuarkManager_ptr mgr)
	{
		if (!mgr->IsValidArea ())
			return;

		if (RemovedIDs_.contains (mgr->GetID ()))
			return;

		Quark2Manager_ [comp.Url_] = mgr;

		auto item = new QStandardItem;
		item->setData (comp.Url_, ViewItemsModel::Role::SourceURL);
		item->setData (mgr->HasSettings (), ViewItemsModel::Role::QuarkHasSettings);
		item->setData (mgr->GetID (), ViewItemsModel::Role::QuarkClass);

		const int pos = PreviousQuarkOrder_.indexOf (mgr->GetID ());
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
			}
			if (!added)
				ViewItemsModel_->appendRow (item);
		}
	}

	QList<QuarkComponent> ViewManager::ScanRootDir (const QDir& dir) const
	{
		QList<QuarkComponent> result;
		for (const auto& entry : dir.entryList (QDir::Dirs | QDir::NoDotAndDotDot | QDir::Readable))
		{
			QDir quarkDir (dir);
			quarkDir.cd (entry);
			if (!quarkDir.exists (entry + ".qml"))
				continue;

			QuarkComponent c;
			c.Url_ = QUrl::fromLocalFile (quarkDir.absoluteFilePath (entry + ".qml"));
			result << c;
		}
		return result;
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
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_SB2");
		settings.beginGroup ("RemovedList");
		settings.setValue ("IDs", QStringList (RemovedIDs_.toList ()));
		settings.endGroup ();
	}

	void ViewManager::LoadRemovedList ()
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_SB2");
		settings.beginGroup ("RemovedList");
		RemovedIDs_ = QSet<QString>::fromList (settings.value ("IDs").toStringList ());
		settings.endGroup ();
	}

	void ViewManager::SaveQuarkOrder ()
	{
		PreviousQuarkOrder_.clear ();
		for (int i = 0; i < ViewItemsModel_->rowCount (); ++i)
		{
			auto item = ViewItemsModel_->item (i);
			PreviousQuarkOrder_ << item->data (ViewItemsModel::Role::QuarkClass).toString ();
		}

		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_SB2");
		settings.beginGroup ("QuarkOrder");
		settings.setValue ("IDs", PreviousQuarkOrder_);
		settings.endGroup ();
	}

	void ViewManager::LoadQuarkOrder ()
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_SB2");
		settings.beginGroup ("QuarkOrder");
		PreviousQuarkOrder_ = settings.value ("IDs").toStringList ();
		settings.endGroup ();
	}

	int ViewManager::GetWindowIndex () const
	{
		auto rootWM = Proxy_->GetRootWindowsManager ();
		return rootWM->GetWindowIndex (Window_);
	}

	void ViewManager::setOrientation (Qt::Orientation orientation)
	{
		switch (orientation)
		{
		case Qt::Vertical:
			View_->resize (View_->minimumSize ());
			View_->setSizePolicy (QSizePolicy::Preferred, QSizePolicy::Expanding);
			View_->rootContext ()->setContextProperty ("viewOrient", "vertical");
			break;
		case Qt::Horizontal:
			View_->resize (View_->minimumSize ());
			View_->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Preferred);
			View_->rootContext ()->setContextProperty ("viewOrient", "horizontal");
			break;
		}
	}

	void ViewManager::handleToolbarTopLevel (bool topLevel)
	{
		if (topLevel)
			return;

		const auto pos = Window_->toolBarArea (Toolbar_);

		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_SB2");
		settings.beginGroup ("Toolbars");
		settings.setValue ("Pos_" + QString::number (GetWindowIndex ()), static_cast<int> (pos));
		settings.endGroup ();
	}
}
}
