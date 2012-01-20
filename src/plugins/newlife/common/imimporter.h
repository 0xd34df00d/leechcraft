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

#pragma once

#include "abstractimporter.h"

namespace LeechCraft
{
namespace NewLife
{
namespace Common
{
	class IMImportPage;

	class IMImporter : public AbstractImporter
	{
		Q_OBJECT

		QString Name_;
		QIcon Icon_;
	protected:
		IMImportPage *Page_;

		IMImporter (const QString&, const QIcon&, IMImportPage*, QObject* = 0);
	public:
		QStringList GetNames () const;
		QList<QIcon> GetIcons () const;
		QList<QWizardPage*> GetWizardPages () const;
	};
}
}
}
