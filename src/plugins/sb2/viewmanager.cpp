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
		for (const auto& cand : Util::GetPathCandidates (Util::SysPath::QML, "quarks"))
			AddRootDir (QDir (cand));

		QDir local = QDir::home ();
		if (local.cd (".leechcraft") &&
			local.cd ("data") &&
			local.cd ("quarks"))
			AddRootDir (local);

		auto pm = Proxy_->GetPluginsManager ();
		for (auto prov : pm->GetAllCastableTo<IQuarkComponentProvider*> ())
			for (auto quark : prov->GetComponents ())
				AddComponent (quark);
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

		if (!mgr->IsValidArea ())
			return;

		Quark2Manager_ [comp.Url_] = mgr;

		auto item = new QStandardItem;
		item->setData (comp.Url_, ViewItemsModel::Role::SourceURL);
		item->setData (mgr->HasSettings (), ViewItemsModel::Role::QuarkHasSettings);
		ViewItemsModel_->appendRow (item);
	}

	void ViewManager::ShowSettings (const QUrl& url)
	{
		auto manager = Quark2Manager_ [url];
		manager->ShowSettings ();
	}

	void ViewManager::AddRootDir (const QDir& dir)
	{
		for (const auto& entry : dir.entryList (QDir::Dirs | QDir::NoDotAndDotDot | QDir::Readable))
		{
			QDir quarkDir (dir);
			quarkDir.cd (entry);
			if (!quarkDir.exists (entry + ".qml"))
				continue;

			QuarkComponent c;
			c.Url_ = QUrl::fromLocalFile (quarkDir.absoluteFilePath (entry + ".qml"));
			AddComponent (c);
		}
	}
}
}
