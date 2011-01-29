/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#ifndef PLUGINS_SEEKTHRU_DESCRIPTION_H
#define PLUGINS_SEEKTHRU_DESCRIPTION_H
#include <QStringList>
#include <QMetaType>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace SeekThru
		{
			struct UrlDescription
			{
				QString Template_;
				QString Type_;
				qint32 IndexOffset_;
				qint32 PageOffset_;
			};

			QDataStream& operator<< (QDataStream&, const UrlDescription&);
			QDataStream& operator>> (QDataStream&, UrlDescription&);

			struct QueryDescription
			{
				enum Role
				{
					RoleRequest,
					RoleExample,
					RoleRelated,
					RoleCorrection,
					RoleSubset,
					RoleSuperset
				};

				Role Role_;
				QString Title_;
				qint32 TotalResults_;
				QString SearchTerms_;
				qint32 Count_;
				qint32 StartIndex_;
				qint32 StartPage_;
				QString Language_;
				QString InputEncoding_;
				QString OutputEncoding_;
			};

			QDataStream& operator<< (QDataStream&, const QueryDescription&);
			QDataStream& operator>> (QDataStream&, QueryDescription&);

			struct Description
			{
				enum SyndicationRight
				{
					SROpen,
					SRLimited,
					SRPrivate,
					SRClosed
				};

				QString ShortName_;
				QString Description_;
				QList<UrlDescription> URLs_;
				QString Contact_;
				QStringList Tags_;
				QString LongName_;
				QList<QueryDescription> Queries_;
				QString Developer_;
				QString Attribution_;
				SyndicationRight Right_;
				bool Adult_;
				QStringList Languages_;
				QStringList InputEncodings_;
				QStringList OutputEncodings_;
			};

			QDataStream& operator<< (QDataStream&, const Description&);
			QDataStream& operator>> (QDataStream&, Description&);
		};
	};
};

Q_DECLARE_METATYPE (LeechCraft::Plugins::SeekThru::Description);

#endif

