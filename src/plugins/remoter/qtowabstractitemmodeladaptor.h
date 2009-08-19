/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#ifndef QTOWABSTRACTITEMMODELADAPTOR_H
#define QTOWABSTRACTITEMMODELADAPTOR_H
#include <map>
#include <WAbstractItemModel>
#include <QAbstractItemModel>

namespace LeechCraft
{
	namespace Util
	{
		class TreeItem;
	};
};

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

	LeechCraft::Util::TreeItem *Root_;

	mutable std::map<LeechCraft::Util::TreeItem*, Wt::WModelIndex> Indexes_;
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

