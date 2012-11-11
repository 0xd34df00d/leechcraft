/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2012  Georg Rudoy
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
#include <interfaces/iquarkcomponentprovider.h>
#include <interfaces/core/ipluginsmanager.h>
#include "sbview.h"
#include "quarkproxy.h"
#include "quarksettingsmanager.h"
#include "widthiconprovider.h"

namespace LeechCraft
{
namespace SB2
{
	namespace
	{
		class ThemeImageProvider : public WidthIconProvider
		{
			ICoreProxy_ptr Proxy_;
		public:
			ThemeImageProvider (ICoreProxy_ptr proxy)
			: Proxy_ (proxy)
			{
			}

			QIcon GetIcon (const QStringList& list)
			{
				return Proxy_->GetIcon (list.value (0));
			}
		};

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
		{
			QDir dir (cand);
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

		auto pm = Proxy_->GetPluginsManager ();
		for (auto prov : pm->GetAllCastableTo<IQuarkComponentProvider*> ())
			for (auto quark : prov->GetComponents ())
				AddComponent (quark);
	}

	void ViewManager::AddComponent (const QuarkComponent& comp)
	{
		qDebug () << Q_FUNC_INFO << "adding" << comp.Url_;
		auto ctx = View_->rootContext ();
		for (const auto& pair : comp.StaticProps_)
			ctx->setContextProperty (pair.first, pair.second);
		for (const auto& pair : comp.DynamicProps_)
			ctx->setContextProperty (pair.first, pair.second);
		for (const auto& pair : comp.ImageProviders_)
			View_->engine ()->addImageProvider (pair.first, pair.second);

		const bool hasSettings = CreateSettings (comp.Url_);

		auto item = new QStandardItem;
		item->setData (comp.Url_, ViewItemsModel::Role::SourceURL);
		item->setData (hasSettings, ViewItemsModel::Role::QuarkHasSettings);
		ViewItemsModel_->appendRow (item);
	}

	void ViewManager::ShowSettings (const QUrl& url)
	{
		if (!Quark2Settings_.contains (url))
			return;

		auto xsd = Quark2Settings_ [url].XSD_;
		xsd->move (QCursor::pos ());
		xsd->show ();
	}

	bool ViewManager::CreateSettings (const QUrl& url)
	{
		if (!url.isLocalFile ())
			return false;

		const auto& localName = url.toLocalFile ();
		const auto& settingsName = localName + ".settings";
		if (!QFile::exists (settingsName))
			return false;

		Util::XmlSettingsDialog_ptr xsd (new Util::XmlSettingsDialog);
		auto sm = new QuarkSettingsManager (url, View_->rootContext ());
		xsd->RegisterObject (sm, settingsName);
		Quark2Settings_ [url] = { xsd, sm };
		return true;
	}
}
}
