/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QVariantList>
#include <QUrl>
#include <QModelIndexList>
#include "constants.h"

class QStandardItemModel;

namespace LC::Poshuku::SpeedDial
{
	typedef QPair<QString, QUrl> Addr_t;
	typedef QList<Addr_t> AddrList_t;

	class CustomSitesManager : public QObject
	{
		Q_OBJECT

		QStandardItemModel * const Model_;
	public:
		CustomSitesManager ();

		QAbstractItemModel* GetModel () const;
		TopList_t GetTopList () const;
	private:
		AddrList_t GetAddresses () const;
		void LoadSettings ();
		void SaveSettings ();

		void Add (const Addr_t&);
	public slots:
		void addRequested (const QString&, const QVariantList&);
		void modifyRequested (const QString&, int, const QVariantList&);
		void removeRequested (const QString&, const QModelIndexList&);
	};
}

Q_DECLARE_METATYPE (LC::Poshuku::SpeedDial::AddrList_t)
