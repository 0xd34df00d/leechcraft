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

#include "imimporter.h"
#include "imimportpage.h"

namespace LeechCraft
{
namespace NewLife
{
namespace Common
{
	IMImporter::IMImporter (const QString& name,
			const QIcon& icon, IMImportPage *page, QObject *parent)
	: AbstractImporter (parent)
	, Name_ (name)
	, Icon_ (icon)
	, Page_ (page)
	{
	}

	QStringList IMImporter::GetNames () const
	{
		return QStringList (Name_);
	}

	QList<QIcon> IMImporter::GetIcons () const
	{
		return QList<QIcon> () << Icon_;
	}

	QList<QWizardPage*> IMImporter::GetWizardPages () const
	{
		QList<QWizardPage*> result;
		result << Page_;
		return result;
	}
}
}
}
