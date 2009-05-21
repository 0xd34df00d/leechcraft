#ifndef PLUGINS_CLEANWEB_CORE_H
#define PLUGINS_CLEANWEB_CORE_H
#include <QObject>
#include <QHash>
#include <QStringList>
#include <QNetworkReply>
#include <QDateTime>
#include <interfaces/iinfo.h>
#include <interfaces/idownload.h>

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
				QObjectList Downloaders_;

				struct PendingJob
				{
					QString FullName_;
					QString FileName_;
					QString Subscr_;
					QUrl URL_;
				};
				QMap<int, PendingJob> PendingJobs_;

				struct SubscriptionData
				{
					QUrl URL_;
					QString Name_;
					QDateTime LastDateTime_;
				};
				QMap<QString, SubscriptionData> Files_;

				Core ();
			public:
				static Core& Instance ();
				void Release ();
				void Handle (DownloadEntity);
				QNetworkReply* Hook (LeechCraft::IHookProxy*,
						QNetworkAccessManager::Operation*,
						QNetworkRequest*,
						QIODevice**);
				bool ShouldReject (const QNetworkRequest&) const;
			private:
				bool Matches (const QString&, const Filter&,
						const QString&, const QString&) const;
				void HandleProvider (QObject*);
				void Parse (const QString&);
				void Update ();
				void Load (const QUrl&, const QString&);
				void Remove (const QString&);
				void WriteSettings ();
				void ReadSettings ();
			private slots:
				void handleJobFinished (int);
				void handleJobError (int, IDownload::Error);
			signals:
				void delegateEntity (const LeechCraft::DownloadEntity&,
						int*, QObject**);
			};
		};
	};
};

#endif

