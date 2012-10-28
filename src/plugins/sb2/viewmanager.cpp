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
#include "sbview.h"
#include "quarkproxy.h"

namespace LeechCraft
{
namespace SB2
{
	namespace
	{
		class ViewItemsModel : public QStandardItemModel
		{
		public:
			enum Role
			{
				SourceURL= Qt::UserRole + 1
			};

			ViewItemsModel (QObject *parent)
			: QStandardItemModel (parent)
			{
				QHash<int, QByteArray> names;
				names [Role::SourceURL] = "sourceURL";
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

		auto item = new QStandardItem;
		item->setData (comp.Url_, ViewItemsModel::Role::SourceURL);
		ViewItemsModel_->appendRow (item);
	}
}
}
