#ifndef JOBHOLDERREPRESENTATION_H
#define JOBHOLDERREPRESENTATION_H
#include <QSortFilterProxyModel>
#include <QQueue>

class QTimer;

class JobHolderRepresentation : public QSortFilterProxyModel
{
	Q_OBJECT

	QModelIndex Selected_;
	QQueue<int> Previous_;
	QTimer *Timer_;
public:
	JobHolderRepresentation (QObject* = 0);
	void SelectionChanged (const QModelIndex&);
protected:
	virtual bool filterAcceptsRow (int, const QModelIndex&) const;
private slots:
	void popRow ();
};

#endif

