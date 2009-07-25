#ifndef TABCONTENTSMANAGER_H
#define TABCONTENTSMANAGER_H
#include <QObject>
#include <QList>

namespace LeechCraft
{
	class TabContents;

	class TabContentsManager : public QObject
	{
		Q_OBJECT

		TabContents *Default_;
		QList<TabContents*> Others_;

		TabContentsManager ();
	public:
		static TabContentsManager& Instance ();

		void SetDefault (TabContents*);
		QList<TabContents*> GetTabs () const;

		void AddNewTab (const QString& = QString ());
		void RemoveTab (TabContents*);
		void MadeCurrent (TabContents*);
	signals:
		void addNewTab (const QString&, QWidget*);
		void removeTab (QWidget*);
	};
};

#endif

