#ifndef TABCONTENTSMANAGER_H
#define TABCONTENTSMANAGER_H
#include <QObject>
#include <QIcon>
#include <QList>

namespace LeechCraft
{
	class TabContents;
	class ViewReemitter;

	class TabContentsManager : public QObject
	{
		Q_OBJECT

		TabContents *Default_;
		TabContents *Current_;
		QList<TabContents*> Others_;

		ViewReemitter *Reemitter_;

		TabContentsManager ();
	public:
		static TabContentsManager& Instance ();

		void SetDefault (TabContents*);
		QList<TabContents*> GetTabs () const;

		void AddNewTab (const QString& = QString ());
		void RemoveTab (TabContents*);
		void MadeCurrent (TabContents*);
		TabContents* GetCurrent () const;
		QObject* GetReemitter () const;
	private:
		void Connect (TabContents*);
	private slots:
		void handleFilterUpdated ();
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

