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
#include <util/sys/paths.h>
#include "ljaccount.h"

namespace LeechCraft
{
namespace Blogique
{
namespace Metida
{
	RecentCommentsSideWidget::RecentCommentsSideWidget (QWidget *parent)
	: QWidget (parent)
	, LJAccount_ (0)
	{
		Ui_.setupUi (this);

		Ui_.View_->setResizeMode (QDeclarativeView::SizeRootObjectToView);
		Ui_.View_->setSource (QUrl::fromLocalFile (Util::GetSysPath (Util::SysPath::QML,
				"blogique/metida", "recentcomments.qml")));
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
	}

}
}
}
