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
#include "interfaces/blogique/iaccount.h"

namespace LeechCraft
{
namespace Blogique
{
namespace Metida
{
	class LJAccount : public QObject
							, public IAccount
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::Blogique::IAccount)

		QObject *ParentBloggingPlatform_;
		QString Name_;
	public:
		LJAccount (const QString& name, QObject *parent = 0);
		QObject* GetObject ();
		QObject* GetParentBloggingPlatform () const;
		QString GetAccountName () const;
		QString GetOurLogin () const;
		void RenameAccount (const QString& name);
		QByteArray GetAccountID () const;
		void OpenConfigurationDialog ();
	signals:
		void accountRenamed (const QString& newName);
	};
}
}
}
