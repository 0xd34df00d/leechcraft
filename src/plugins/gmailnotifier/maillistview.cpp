/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "maillistview.h"

#include <QQuickWidget>
#include <QQmlContext>
#include <QQmlEngine>
#include <QStandardItemModel>
#include <QtDebug>
#include <util/sys/paths.h>
#include <util/gui/unhoverdeletemixin.h>
#include <util/qml/colorthemeproxy.h>
#include <util/models/rolenamesmixin.h>

namespace LC
{
namespace GmailNotifier
{
	namespace
	{
		class MailListModel : public Util::RoleNamesMixin<QStandardItemModel>
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
			: RoleNamesMixin<QStandardItemModel> (parent)
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
	: QQuickWidget (parent)
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
				dateString = QLocale {}.toString (info.Modified_.date (), QLocale::ShortFormat);
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
