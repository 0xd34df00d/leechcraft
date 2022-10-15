/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QSortFilterProxyModel>

namespace LC
{
namespace Azoth
{
	class SortFilterProxyModel : public QSortFilterProxyModel
	{
		Q_OBJECT

		bool ShowOffline_ = true;
		bool MUCMode_ = false;
		bool OrderByStatus_ = true;
		bool HideMUCParts_ = false;
		bool ShowSelfContacts_ = true;
		bool HideErroring_ = true;
		QObject *MUCEntry_ = nullptr;
	public:
		SortFilterProxyModel (QObject* = nullptr);

		void SetMUCMode (bool);
		bool IsMUCMode () const;
		void SetMUC (QObject*);
	public slots:
		void showOfflineContacts (bool);
	private slots:
		void handleStatusOrderingChanged ();
		void handleHideMUCPartsChanged ();
		void handleShowSelfContactsChanged ();
		void handleHideErrorContactsChanged ();
		void handleMUCDestroyed ();
	protected:
		bool filterAcceptsRow (int, const QModelIndex&) const override;
		bool lessThan (const QModelIndex&, const QModelIndex&) const override;
	private:
		bool FilterAcceptsMucMode (int, const QModelIndex&) const;
		bool FilterAcceptsNonMucMode (int, const QModelIndex&) const;
	signals:
		void mucMode ();
		void wholeMode ();
	};
}
}
