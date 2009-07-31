#ifndef VIEWREEMITTER_H
#define VIEWREEMITTER_H
#include <QObject>

class QModelIndex;
class QItemSelection;
class QTreeView;

namespace LeechCraft
{
	class TabContents;

	class ViewReemitter : public QObject
	{
		Q_OBJECT
	public:
		ViewReemitter (QObject* = 0);

		void Connect (TabContents*);
		void ConnectModelSpecific (TabContents*);
	public slots:
		void handle_activated (const QModelIndex&);
		void handle_clicked (const QModelIndex&);
		void handle_doubleClicked (const QModelIndex&);
		void handle_entered (const QModelIndex&);
		void handle_pressed (const QModelIndex&);
		void handleViewportEntered ();

		void handle_currentChanged (const QModelIndex&, const QModelIndex&);
		void handle_currentColumnChanged (const QModelIndex&, const QModelIndex&);
		void handle_currentRowChanged (const QModelIndex&, const QModelIndex&);
		void handleSelectionChanged (const QItemSelection&, const QItemSelection&);
	signals:
		void activated (const QModelIndex&, QTreeView*);
		void clicked (const QModelIndex&, QTreeView*);
		void doubleClicked (const QModelIndex&, QTreeView*);
		void entered (const QModelIndex&, QTreeView*);
		void pressed (const QModelIndex&, QTreeView*);
		void viewportEntered (QTreeView*);

		void currentChanged (const QModelIndex&, const QModelIndex&, QTreeView*);
		void currentColumnChanged (const QModelIndex&, const QModelIndex&, QTreeView*);
		void currentRowChanged (const QModelIndex&, const QModelIndex&, QTreeView*);
		void selectionChanged (const QItemSelection&, const QItemSelection&, QTreeView*);
	};
};

#endif

