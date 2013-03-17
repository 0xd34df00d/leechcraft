/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin
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

#include "recentcommentssidewidget.h"
#include <QtDebug>
#include <QUrl>
#include <QDeclarativeContext>
#include <QGraphicsObject>
#include <util/sys/paths.h>
#include <util/qml/colorthemeproxy.h>
#include <util/util.h>
#include "core.h"
#include "ljaccount.h"
#include "recentcommentsmodel.h"
#include "recentcommentsview.h"

namespace LeechCraft
{
namespace Blogique
{
namespace Metida
{
	RecentCommentsSideWidget::RecentCommentsSideWidget (QWidget *parent)
	: QWidget (parent)
	, LJAccount_ (0)
	, RecentCommentsModel_ (new RecentCommentsModel (this))
	{
		Ui_.setupUi (this);

		auto view = new RecentCommentsView (this);
		Ui_.GridLayout_->addWidget (view);

		view->setResizeMode (QDeclarativeView::SizeRootObjectToView);
		view->rootContext ()->setContextProperty ("colorProxy",
				new Util::ColorThemeProxy (Core::Instance ()
						.GetCoreProxy ()->GetColorThemeManager ()));
		view->rootContext ()->setContextProperty ("recentCommentsModel",
				RecentCommentsModel_);
		view->rootContext ()->setContextProperty ("recentCommentsView",
				view);
		view->setSource (QUrl::fromLocalFile (Util::GetSysPath (Util::SysPath::QML,
				"blogique/metida", "recentcomments.qml")));

		connect (view->rootObject (),
				SIGNAL (linkActivated (QString)),
				this,
				SLOT (handleLinkActivated (QString)));
	}

	QString RecentCommentsSideWidget::GetName () const
	{
		return tr ("Recent comments");
	}

	SideWidgetType RecentCommentsSideWidget::GetWidgetType () const
	{
		return SideWidgetType::CustomSideWidget;
	}

	QVariantMap RecentCommentsSideWidget::GetPostOptions () const
	{
		return QVariantMap ();
	}

	void RecentCommentsSideWidget::SetPostOptions (const QVariantMap&)
	{
	}

	QVariantMap RecentCommentsSideWidget::GetCustomData () const
	{
		return QVariantMap ();
	}

	void RecentCommentsSideWidget::SetCustomData (const QVariantMap&)
	{
	}

	void RecentCommentsSideWidget::SetAccount (QObject* accountObj)
	{
		LJAccount_ = qobject_cast<LJAccount*> (accountObj);
		if (LJAccount_)
			connect (accountObj,
					SIGNAL (gotRecentComments (QList<LJCommentEntry>)),
					this,
					SLOT (handleGotRecentComents (QList<LJCommentEntry>)),
					Qt::UniqueConnection);
	}

	void RecentCommentsSideWidget::handleGotRecentComents (const QList<LJCommentEntry>& comments)
	{
		for (auto comment : comments)
		{
			QStandardItem *item = new QStandardItem;
			item->setData (comment.NodeSubject_, RecentCommentsModel::NodeSubject);
			item->setData (comment.NodeUrl_, RecentCommentsModel::NodeUrl);
			item->setData (comment.Text_, RecentCommentsModel::CommentBody);
			item->setData ("", RecentCommentsModel::CommentBodyUrl);
			item->setData (tr ("by %1 on %2")
						.arg (comment.PosterName_.isEmpty () ?
							tr ("Anonymous") :
							comment.PosterName_)
						.arg (comment.PostingDate_.toString (Qt::DefaultLocaleShortDate)),
					RecentCommentsModel::CommentInfo);

			RecentCommentsModel_->appendRow (item);
		}
	}

	void RecentCommentsSideWidget::handleLinkActivated (const QString& link)
	{
		Core::Instance ().SendEntity (Util::MakeEntity (link,
				QString (),
				OnlyHandle | FromUserInitiated));
	}

}
}
}
