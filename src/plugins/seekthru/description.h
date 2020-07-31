/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QStringList>
#include <QString>
#include <QVariantMap>
#include <QUrl>
#include <QMetaType>

namespace LC::SeekThru
{
	struct UrlDescription
	{
		QString Template_;
		QString Type_;
		qint32 IndexOffset_;
		qint32 PageOffset_;

		QUrl MakeUrl (const QString&, const QHash<QString, QVariant>&) const;
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
		enum class SyndicationRight
		{
			Open,
			Limited,
			Private,
			Closed
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
}

Q_DECLARE_METATYPE (LC::SeekThru::Description)
