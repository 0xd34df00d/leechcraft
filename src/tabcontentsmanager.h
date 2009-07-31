#ifndef TABCONTENTSMANAGER_H
#define TABCONTENTSMANAGER_H
#include <QObject>
#include <QIcon>
#include <QList>

namespace LeechCraft
{
	class TabContents;

	class TabContentsManager : public QObject
	{
		Q_OBJECT

		TabContents *Default_;
		TabContents *Current_;
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
		void changeTabName (QWidget*, const QString&);
		void changeTabIcon (QWidget*, const QIcon&);
		void changeTooltip (QWidget*, QWidget*);
		void statusBarChanged (QWidget*, const QString&);
		void raiseTab (QWidget*);
	};
};

#endif

