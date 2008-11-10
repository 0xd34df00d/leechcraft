#ifndef QTOWABSTRACTITEMMODELADAPTOR_H
#define QTOWABSTRACTITEMMODELADAPTOR_H
#include <QAbstractItemModel>
#include <WAbstractItemModel>

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
public:
	QToWAbstractItemModelAdaptor (QAbstractItemModel*, WObject* = 0);
	virtual ~QToWAbstractItemModelAdaptor ();

	int columnCount (const Wt::WModelIndex& = Wt::WModelIndex ()) const;
	int rowCount (const Wt::WModelIndex& = Wt::WModelIndex ()) const;
	Wt::WModelIndex parent (const Wt::WModelIndex&) const;
	boost::any data (const Wt::WModelIndex&, int = Wt::DisplayRole) const;
private:
	boost::any Convert (const QVariant&) const;
	QVariant Convert (const boost::any&) const;
	QModelIndex Convert (const Wt::WModelIndex&) const;
	Wt::WModelIndex Convert (const QModelIndex&) const;
};

#endif

