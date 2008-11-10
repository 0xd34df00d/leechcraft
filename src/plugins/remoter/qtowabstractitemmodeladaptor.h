#ifndef QTOWABSTRACTITEMMODELADAPTOR_H
#define QTOWABSTRACTITEMMODELADAPTOR_H
#include <map>
#include <QAbstractItemModel>
#include <WAbstractItemModel>

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
	Wt::WModelIndex parent (const Wt::WModelIndex&) const;
	boost::any data (const Wt::WModelIndex&, int = Wt::DisplayRole) const;
	boost::any headerData (int, Wt::Orientation, int) const;
	Wt::WModelIndex index (int, int,
			const Wt::WModelIndex& = Wt::WModelIndex ()) const;
private:
	boost::any Convert (const QVariant&) const;
	QVariant Convert (const boost::any&) const;
	QModelIndex Convert (const Wt::WModelIndex&) const;
	Wt::WModelIndex Convert (const QModelIndex&) const;
};

#endif

