#ifndef PLUGINS_DEADLYRICS_CORE_H
#define PLUGINS_DEADLYRICS_CORE_H
#include <QAbstractItemModel>
#include "searcher.h"

class QNetworkAccessManager;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace DeadLyrics
		{
			class Core : public QObject
			{
				Q_OBJECT

				searchers_t Searchers_;
				QNetworkAccessManager *Manager_;

				Core ();
			public:
				static Core& Instance ();
				void Release ();

				void SetNetworkAccessManager (QNetworkAccessManager*);
				QNetworkAccessManager* GetNetworkAccessManager () const;
				QStringList GetCategories () const;

				/** Returns all the searches for the given category. It's assumed
				 * that different calls to this function with the same category
				 * return the same searchers in the same order.
				 *
				 * @param[in] category The category for which one wants to get the
				 * searchers.
				 * @return The searchers for the passed category.
				 */
				searchers_t GetSearchers (const QString& category) const;
			};
		};
	};
};

#endif

