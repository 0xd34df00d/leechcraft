#ifndef PLUGINS_DEADLYRICS_FINDPROXY_H
#define PLUGINS_DEADLYRICS_FINDPROXY_H
#include <vector>
#include <QAbstractItemModel>
#include <interfaces/ifinder.h>
#include "searcher.h"

class QTextEdit;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace DeadLyrics
		{
			class FindProxy : public QAbstractItemModel
							, public IFindProxy
			{
				Q_OBJECT
				Q_INTERFACES (IFindProxy);

				LeechCraft::Request Request_;
				std::vector<QByteArray> Hashes_;
				lyrics_t Lyrics_;
				QTextEdit *LyricsHolder_;
				QString ErrorString_;
				bool FetchedSomething_;

				FindProxy (const FindProxy&);
				FindProxy& operator= (const FindProxy&);
			public:
				FindProxy (const LeechCraft::Request&, QObject* = 0);
				virtual ~FindProxy ();

				QAbstractItemModel* GetModel ();

				int columnCount (const QModelIndex&) const;
				QVariant data (const QModelIndex&, int) const;
				QModelIndex index (int, int, const QModelIndex& = QModelIndex ()) const;
				QModelIndex parent (const QModelIndex&) const;
				int rowCount (const QModelIndex&) const;
			private slots:
				void handleTextFetched (const LeechCraft::Plugins::DeadLyrics::Lyrics&, const QByteArray&);
				void handleError (const QString&);
			};
		};
	};
};

#endif

