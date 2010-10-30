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
#include <QUrl>
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
				QUrl PageURL_;
				QString FormID_;
				QString Name_;
				QString Type_;
				QString Value_;
			};

			typedef QList<ElementData> ElementsData_t;

			QDebug& operator<< (QDebug&, const ElementData&);

			/** Holds information about all the forms on a page.
			 *
			 * The key of the map is the name of the `input' element,
			 * whereas value is the ElementData structure with the
			 * information about that element.
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

Q_DECLARE_METATYPE (LeechCraft::Plugins::Poshuku::ElementData);
Q_DECLARE_METATYPE (LeechCraft::Plugins::Poshuku::ElementsData_t);
Q_DECLARE_METATYPE (LeechCraft::Plugins::Poshuku::PageFormsData_t);

#endif

