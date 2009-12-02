/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2009  Georg Rudoy
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

#ifndef PLUGINS_NEWLIFE_KTORRENTIMPORTER_H
#define PLUGINS_NEWLIFE_KTORRENTIMPORTER_H
#include "abstractimporter.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace NewLife
		{
			class KTorrentImportPage;

			class KTorrentImporter : public AbstractImporter
			{
				Q_OBJECT

				KTorrentImportPage *ImportPage_;
			public:
				KTorrentImporter (QWidget* = 0);
				virtual QStringList GetNames () const;
				virtual QList<QWizardPage*> GetWizardPages () const;
			};
		};
	};
};

#endif

