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

#include <QObject>
#include <interfaces/blogique/iprofile.h>
#include "profiletypes.h"

namespace LeechCraft
{
namespace Blogique
{
namespace Metida
{
	class ProfileWidget;

	class LJProfile : public QObject
					, public IProfile
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::Blogique::IProfile);

		QObject *ParentAccount_;
		LJProfileData ProfileData_;
	public:
		LJProfile (QObject *parentAccount, QObject *parent = 0);
		QWidget* GetProfileWidget ();
		LJProfileData GetProfileData () const;
		QObject* GetParentAccount () const;
	private:
		void SaveAvatar (QUrl url = QUrl ());

	public slots:
		void handleProfileUpdate (const LJProfileData& profile);
	private slots:
		void handleAvatarDownloadFinished ();

	signals:
		void profileUpdated ();
	};
}
}
}
