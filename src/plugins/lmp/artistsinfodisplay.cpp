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

#include "artistsinfodisplay.h"
#include <algorithm>
#include <QStandardItemModel>
#include <QGraphicsObject>
#include <QDeclarativeContext>
#include <QDeclarativeImageProvider>
#include <QDeclarativeEngine>
#include <util/util.h>
#include "core.h"
#include "localcollection.h"
#include "similarmodel.h"
#include "sysiconsprovider.h"

namespace LeechCraft
{
namespace LMP
{
	ArtistsInfoDisplay::ArtistsInfoDisplay (QWidget *parent)
	: QDeclarativeView (parent)
	, Model_ (new SimilarModel (this))
	{
		engine ()->addImageProvider ("sysIcons", new SysIconProvider (Core::Instance ().GetProxy ()));
		rootContext ()->setContextProperty ("similarModel", Model_);
		setSource (QUrl ("qrc:/lmp/resources/qml/SimilarView.qml"));

		connect (rootObject (),
				SIGNAL (bookmarkArtistRequested (QString, QString, QString)),
				this,
				SLOT (handleBookmark (QString, QString, QString)));
		connect (rootObject (),
				SIGNAL (linkActivated (QString)),
				this,
				SLOT (handleLink (QString)));
	}

	void ArtistsInfoDisplay::SetSimilarArtists (Media::SimilarityInfos_t infos)
	{
		Model_->clear ();

		std::sort (infos.begin (), infos.end (),
				[] (const Media::SimilarityInfo& left, const Media::SimilarityInfo& right)
					{ return left.Similarity_ > right.Similarity_; });

		Q_FOREACH (const Media::SimilarityInfo& info, infos)
		{
			auto item = SimilarModel::ConstructItem (info.Artist_);

			QString simStr;
			if (info.Similarity_ > 0)
				simStr = tr ("Similarity: %1%")
					.arg (info.Similarity_);
			else if (!info.SimilarTo_.isEmpty ())
				simStr = tr ("Similar to: %1")
					.arg (info.SimilarTo_.join ("; "));
			if (!simStr.isEmpty ())
				item->setData (simStr, SimilarModel::Role::Similarity);

			Model_->appendRow (item);
		}
	}

	void ArtistsInfoDisplay::handleBookmark (const QString& name, const QString& page, const QString& tags)
	{
		auto e = Util::MakeEntity (tr ("Check out \"%1\"").arg (name),
				QString (),
				FromUserInitiated | OnlyHandle,
				"x-leechcraft/todo-item");
		e.Additional_ ["TodoBody"] = tags + "<br />" + QString ("<a href='%1'>%1</a>").arg (page);
		e.Additional_ ["Tags"] = QStringList ("music");
		Core::Instance ().SendEntity (e);
	}

	void ArtistsInfoDisplay::handleLink (const QString& link)
	{
		Core::Instance ().SendEntity (Util::MakeEntity (QUrl (link),
					QString (),
					FromUserInitiated | OnlyHandle));
	}
}
}
