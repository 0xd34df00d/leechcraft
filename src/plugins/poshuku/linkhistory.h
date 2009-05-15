#ifndef PLUGINS_POSHUKU_LINKHISTORY_H
#define PLUGINS_POSHUKU_LINKHISTORY_H
#include <QWebHistoryInterface>
#include <QSet>

namespace LeechCraft
{
	namespace Poshuku
	{
		class LinkHistory : public QWebHistoryInterface
		{
			Q_OBJECT

			QSet<QString> History_;
		public:
			LinkHistory (QObject* = 0);
			void addHistoryEntry (const QString& url);
			bool historyContains (const QString& url) const;
		};
	};
};

#endif

