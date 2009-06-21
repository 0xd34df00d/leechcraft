#ifndef PLUGINS_POSHUKU_PLUGINS_CLEANWEB_CORE_H
#define PLUGINS_POSHUKU_PLUGINS_CLEANWEB_CORE_H
#include <QAbstractItemModel>
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
		namespace Poshuku
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

					struct SubscriptionData
					{
						/// The URL of the subscription.
						QUrl URL_;
						/** The name of the subscription as provided by the abp:
						 * link.
						 */
						QString Name_;
						/// This is the name of the file inside the
						//~/.leechcraft/cleanweb/.
						QString Filename_;
						/// The date/time of last update.
						QDateTime LastDateTime_;
					};

					struct Filter
					{
						QStringList ExceptionStrings_;
						QStringList FilterStrings_;
						QHash<QString, FilterOption> Options_;
						QHash<QString, QRegExp> RegExps_;

						SubscriptionData SD_;
					};

					class Core : public QAbstractItemModel
					{
						Q_OBJECT

						QList<Filter> Filters_;
						QObjectList Downloaders_;
						QStringList HeaderLabels_;

						struct PendingJob
						{
							QString FullName_;
							QString FileName_;
							QString Subscr_;
							QUrl URL_;
						};
						QMap<int, PendingJob> PendingJobs_;

						Core ();
					public:
						static Core& Instance ();
						void Release ();

						int columnCount (const QModelIndex& = QModelIndex ()) const;
						QVariant data (const QModelIndex&, int) const;
						QVariant headerData (int, Qt::Orientation, int) const;
						QModelIndex index (int, int, const QModelIndex& = QModelIndex ()) const;
						QModelIndex parent (const QModelIndex&) const;
						int rowCount (const QModelIndex& = QModelIndex ()) const;

						bool CouldHandle (const DownloadEntity&) const;
						void Handle (DownloadEntity);
						QAbstractItemModel* GetModel ();
						void Remove (const QModelIndex&);
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

						/** Loads the subscription from the url with the name
						 * subscrName. Returns true if the load delegation was
						 * successful, otherwise returns false.
						 */
						bool Load (const QUrl& url, const QString& subscrName);

						/** Returns the subscription at
						 * ~/.leechcraft/cleanweb/name.
						 */
						void Remove (const QString& name);
						void WriteSettings ();
						void ReadSettings ();
						bool AssignSD (const SubscriptionData&);
					private slots:
						void update ();
						void handleJobFinished (int);
						void handleJobError (int, IDownload::Error);
					signals:
						void delegateEntity (const LeechCraft::DownloadEntity&,
								int*, QObject**);
					};
				};
			};
		};
	};
};

#endif

