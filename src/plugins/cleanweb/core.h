#ifndef PLUGINS_CLEANWEB_CORE_H
#define PLUGINS_CLEANWEB_CORE_H
#include <QObject>
#include <QHash>
#include <QStringList>
#include <QNetworkReply>
#include <interfaces/iinfo.h>

class QNetworkRequest;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace CleanWeb
		{
			struct FilterOption
			{
				Qt::CaseSensitivity Case_;
				enum MatchType
				{
					MTWildcard_,
					MTRegexp_
				};
				MatchType MatchType_;
				QStringList Domains_;
				QStringList NotDomains_;

				FilterOption ();
			};

			bool operator== (const FilterOption&, const FilterOption&);
			bool operator!= (const FilterOption&, const FilterOption&);

			struct Filter
			{
				QStringList ExceptionStrings_;
				QStringList FilterStrings_;
				QHash<QString, FilterOption> Options_;
				QHash<QString, QRegExp> RegExps_;
			};

			class Core : public QObject
			{
				Q_OBJECT

				QList<Filter> Filters_;

				Core ();
			public:
				static Core& Instance ();
				void Release ();
				QNetworkReply* Hook (LeechCraft::IHookProxy*,
						QNetworkAccessManager::Operation*,
						QNetworkRequest*,
						QIODevice**);
				bool ShouldReject (const QNetworkRequest&) const;
			private:
				bool Matches (const QString&, const Filter&,
						const QString&, const QString&) const;
			};
		};
	};
};

#endif

