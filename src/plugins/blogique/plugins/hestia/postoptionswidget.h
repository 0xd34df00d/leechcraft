/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
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

#pragma once

#include <QWidget>
#include <interfaces/blogique/iblogiquesidewidget.h>
#include <interfaces/blogique/ipostoptionswidget.h>
#include "ui_postoptionswidget.h"

namespace LeechCraft
{
namespace Blogique
{
namespace Hestia
{
	class LocalBlogAccount;

	class PostOptionsWidget : public QWidget
							, public IBlogiqueSideWidget
							, public IPostOptionsWidget
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::Blogique::IBlogiqueSideWidget
				LeechCraft::Blogique::IPostOptionsWidget)

		Ui::PostOptions Ui_;
		LocalBlogAccount *Account_;

	public:
		PostOptionsWidget (QWidget *parent = 0);

		QString GetName () const;
		SideWidgetType GetWidgetType () const;
		QVariantMap GetPostOptions () const;
		void SetPostOptions (const QVariantMap& map);
		QVariantMap GetCustomData () const;
		void SetCustomData (const QVariantMap& map);
		void SetAccount (QObject *account);

		QStringList GetTags () const;
		void SetTags (const QStringList& tags);
		QDateTime GetPostDate () const;
		void SetPostDate (const QDateTime& date);

	private slots:
		void on_CurrentTime__released ();
	};
}
}
}

