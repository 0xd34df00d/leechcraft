#ifndef PLUGINS_SEEKTHRU_SEARCHHANDLER_H
#define PLUGINS_SEEKTHRU_SEARCHHANDLER_H
#include <QAbstractItemModel>
#include <boost/shared_ptr.hpp>
#include <QUrl>
#include <interfaces/ifinder.h>
#include <interfaces/structures.h>
#include "description.h"

class QToolBar;
class QAction;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace SeekThru
		{
			class SelectableBrowser;
			/** This class performs search on a single category with a single search
			 * provider.
			 */
			class SearchHandler : public QAbstractItemModel
			{
				Q_OBJECT

				static const QString OS_;

				Description D_;

				QString SearchString_;
				struct Result
				{
					enum Type
					{
						TypeRSS,
						TypeAtom,
						TypeHTML
					};

					Type Type_;
					int TotalResults_;
					int StartIndex_;
					int ItemsPerPage_;
					QString Response_;
					QString Filename_;
					QUrl RequestURL_;
				};

				QList<Result> Results_;
				QMap<int, Result> Jobs_;
				QList<QObject*> Downloaders_;
				boost::shared_ptr<SelectableBrowser> Viewer_;
				boost::shared_ptr<QToolBar> Toolbar_;
				boost::shared_ptr<QAction> Action_;
			public:
				SearchHandler (const Description&);

				virtual int columnCount (const QModelIndex& = QModelIndex ()) const;
				virtual QVariant data (const QModelIndex&, int = Qt::DisplayRole) const;
				virtual Qt::ItemFlags flags (const QModelIndex&) const;
				virtual QVariant headerData (int, Qt::Orientation, int = Qt::DisplayRole) const;
				virtual QModelIndex index (int, int, const QModelIndex& = QModelIndex()) const;
				virtual QModelIndex parent (const QModelIndex&) const;
				virtual int rowCount (const QModelIndex& = QModelIndex ()) const;

				void Start (const LeechCraft::Request&);
			private slots:
				void handleJobFinished (int);
				void handleJobError (int);
				void subscribe ();
			private:
				void HandleProvider (QObject*);
			signals:
				void delegateEntity (const LeechCraft::DownloadEntity&,
						int*, QObject**);
				void gotEntity (const LeechCraft::DownloadEntity&);
				void error (const QString&);
				void warning (const QString&);
			};

			typedef boost::shared_ptr<SearchHandler> SearchHandler_ptr;
		};
	};
};

#endif

