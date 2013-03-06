/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#include "maillistview.h"
#include <QDeclarativeContext>
#include <QDeclarativeEngine>
#include <QStandardItemModel>
#include <QtDebug>
#include <util/sys/paths.h>
#include <util/gui/unhoverdeletemixin.h>
#include <util/qml/colorthemeproxy.h>

namespace LeechCraft
{
namespace GmailNotifier
{
	namespace
	{
		class MailListModel : public QStandardItemModel
		{
		public:
			enum Roles
			{
				Subject = Qt::UserRole + 1,
				Summary,
				ModifiedDate,
				AuthorName,
				AuthorEmail
			};

			MailListModel (QObject *parent)
			: QStandardItemModel (parent)
			{
				QHash<int, QByteArray> roleNames;
				roleNames [Subject] = "subject";
				roleNames [Summary] = "summary";
				roleNames [ModifiedDate] = "modifiedDate";
				roleNames [AuthorName] = "authorName";
				roleNames [AuthorEmail] = "authorEmail";
				setRoleNames (roleNames);
			}
		};
	}

	MailListView::MailListView (const ConvInfos_t& infos, ICoreProxy_ptr proxy, QWidget *parent)
	: QDeclarativeView (parent)
	, Model_ (new MailListModel (this))
	{
		new Util::UnhoverDeleteMixin (this);

		const auto& file = Util::GetSysPath (Util::SysPath::QML, "gmailnotifier", "MailListView.qml");
		if (file.isEmpty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "file not found";
			deleteLater ();
			return;
		}

		const auto& now = QDateTime::currentDateTime ();
		for (const auto& info : infos)
		{
			auto item = new QStandardItem;
			item->setData (info.Title_, MailListModel::Roles::Subject);
			item->setData (info.AuthorEmail_, MailListModel::Roles::AuthorEmail);
			item->setData (info.AuthorName_, MailListModel::Roles::AuthorName);
			item->setData (info.Summary_, MailListModel::Roles::Summary);

			QString dateString;
			if (info.Modified_.secsTo (now) < 12 * 60 * 60)
				dateString = info.Modified_.time ().toString ("hh:mm");
			else if (now.date ().day () == info.Modified_.date ().day () - 1)
				dateString = tr ("Yesterday, %1").arg (info.Modified_.time ().toString ());
			else if (now.date ().year () == info.Modified_.date ().year ())
				dateString = info.Modified_.date ().toString ("d MMM");
			else
				dateString = info.Modified_.date ().toString (Qt::DefaultLocaleShortDate);
			item->setData (dateString, MailListModel::Roles::ModifiedDate);
			Model_->appendRow (item);
		}

		setStyleSheet ("background: transparent");
		setWindowFlags (Qt::ToolTip);
		setAttribute (Qt::WA_TranslucentBackground);

		rootContext ()->setContextProperty ("mailListModel", Model_);
		rootContext ()->setContextProperty ("colorProxy",
				new Util::ColorThemeProxy (proxy->GetColorThemeManager (), this));
		setSource (QUrl::fromLocalFile (file));
	}
}
}
