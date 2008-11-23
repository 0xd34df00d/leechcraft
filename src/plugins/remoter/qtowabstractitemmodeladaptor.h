#ifndef QTOWABSTRACTITEMMODELADAPTOR_H
#define QTOWABSTRACTITEMMODELADAPTOR_H
#include <map>
#include <WAbstractItemModel>
#include <QAbstractItemModel>

class TreeItem;

/** @brief Adaptor from QAbstractItemModel to Wt::WAbstractItemModel.
 *
 * Allows one to use QAbstractItemModel where Wt::WAbstractItemModel is
 * expected.
 */
class QToWAbstractItemModelAdaptor : public QObject
								   , public Wt::WAbstractItemModel
{
	Q_OBJECT

	QAbstractItemModel *Model_;

	TreeItem *Root_;

	mutable std::map<TreeItem*, Wt::WModelIndex> Indexes_;
public:
	QToWAbstractItemModelAdaptor (QAbstractItemModel*, WObject* = 0);
	virtual ~QToWAbstractItemModelAdaptor ();

	int columnCount (const Wt::WModelIndex& = Wt::WModelIndex ()) const;
	int rowCount (const Wt::WModelIndex& = Wt::WModelIndex ()) const;
	int flags (const Wt::WModelIndex&) const;
	bool hasChildren (const Wt::WModelIndex&) const;
	Wt::WModelIndex parent (const Wt::WModelIndex&) const;
	boost::any data (const Wt::WModelIndex&, int = Wt::DisplayRole) const;
	boost::any headerData (int, Wt::Orientation, int) const;
	Wt::WModelIndex index (int, int,
			const Wt::WModelIndex& = Wt::WModelIndex ()) const;
private Q_SLOTS:
	void reColumnsAboutToBeInserted (const QModelIndex&, int, int);
	void reColumnsAboutToBeRemoved (const QModelIndex&, int, int);
	void reColumnsInserted (const QModelIndex&, int, int);
	void reColumnsRemoved (const QModelIndex&, int, int);
	void reDataChanged (const QModelIndex&, const QModelIndex&);
	void reHeaderDataChanged (Qt::Orientation, int, int);
	void reInvalidate ();
	void reLayoutAboutToBeChanged ();
	void reLayoutChanged ();
	void reModelAboutToBeReset ();
	void reModelReset ();
	void reRowsAboutToBeInserted (const QModelIndex&, int, int);
	void reRowsAboutToBeRemoved (const QModelIndex&, int, int);
	void reRowsInserted (const QModelIndex&, int, int);
	void reRowsRemoved (const QModelIndex&, int, int);
private:
	QModelIndex Convert (const Wt::WModelIndex&) const;
	Wt::WModelIndex Convert (const QModelIndex&) const;
};

#endif

