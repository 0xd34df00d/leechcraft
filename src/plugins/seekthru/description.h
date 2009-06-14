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

