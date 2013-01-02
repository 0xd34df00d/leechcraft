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
#include <util/sys/paths.h>
#include <util/qml/colorthemeproxy.h>
#include <interfaces/iquarkcomponentprovider.h>
#include <interfaces/core/ipluginsmanager.h>
#include "sbview.h"
#include "quarkproxy.h"
#include "themeimageprovider.h"
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
				QuarkHasSettings
			};

			ViewItemsModel (QObject *parent)
			: QStandardItemModel (parent)
			{
				QHash<int, QByteArray> names;
				names [Role::SourceURL] = "sourceURL";
				names [Role::QuarkHasSettings] = "quarkHasSettings";
				setRoleNames (names);
			}
		};
	}

	ViewManager::ViewManager (ICoreProxy_ptr proxy, QObject *parent)
	: QObject (parent)
	, Proxy_ (proxy)
	, ViewItemsModel_ (new ViewItemsModel (this))
	, View_ (new SBView)
	{
		const auto& file = Util::GetSysPath (Util::SysPath::QML, "sb2", "SideView.qml");
		if (file.isEmpty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "file not found";
			return;
		}

		View_->rootContext ()->setContextProperty ("itemsModel", ViewItemsModel_);
		View_->rootContext ()->setContextProperty ("quarkProxy", new QuarkProxy (this, this));
		View_->rootContext ()->setContextProperty ("colorProxy",
				new Util::ColorThemeProxy (proxy->GetColorThemeManager (), this));
		View_->engine ()->addImageProvider (ImageProviderID, new ThemeImageProvider (proxy));
		View_->setSource (QUrl::fromLocalFile (file));
	}

	SBView* ViewManager::GetView () const
	{
		return View_;
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
		RemovedIDs_ << mgr->GetID ();
	}

	void ViewManager::UnhideQuark (const QuarkComponent& component, QuarkManager_ptr manager)
	{
		if (!manager)
			return;

		RemovedIDs_.remove (manager->GetID ());

		AddComponent (component, manager);
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
		return Quark2Manager_.keys ();
	}

	void ViewManager::AddComponent (const QuarkComponent& comp)
	{
		QuarkManager_ptr mgr;
		try
		{
			mgr.reset (new QuarkManager (comp, this));
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
		ViewItemsModel_->appendRow (item);
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
}
}
