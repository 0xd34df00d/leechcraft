/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#ifndef PLUGINS_POSHUKU_PLUGINS_POSHUKU_PAGEFORMSDATA_H
#define PLUGINS_POSHUKU_PLUGINS_POSHUKU_PAGEFORMSDATA_H
#include <QMap>
#include <QString>
#include <QVariant>

class QDebug;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			struct ElementData
			{
				int FormIndex_;
				QString Name_;
				QString Type_;
				QVariant Value_;
			};

			QDebug& operator<< (QDebug&, const ElementData&);

			/** Holds information about all the elements on the single form.
			 */
			typedef QList<ElementData> ElementsData_t;

			/** Holds information about all the forms/pages, identified by their
			 * URL.
			 */
			typedef QMap<QString, ElementsData_t> PageFormsData_t;

			struct ElemFinder
			{
				const QString& ElemName_;
				const QString& ElemType_;

				ElemFinder (const QString& en, const QString& et)
				: ElemName_ (en)
				, ElemType_ (et)
				{
				}

				inline bool operator() (const ElementData& ed) const
				{
					return ed.Name_ == ElemName_ &&
						ed.Type_ == ElemType_;
				}
			};
		};
	};
};

#endif

