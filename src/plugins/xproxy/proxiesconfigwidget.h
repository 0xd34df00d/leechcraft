/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#pragma once

#include <QWidget>
#include <QNetworkProxy>
#include "ui_proxiesconfigwidget.h"

class QStandardItemModel;

namespace LeechCraft
{
namespace XProxy
{
	struct ReqTarget
	{
		QRegExp Host_;
		int Port_;
		QStringList Protocols_;
	};
	struct Proxy
	{
		QNetworkProxy::ProxyType Type_;
		QString Host_;
		int Port_;
		QString User_;
		QString Pass_;

		operator QNetworkProxy () const;
	};
	typedef QPair<ReqTarget, Proxy> Entry_t;

	QDataStream& operator<< (QDataStream&, const Proxy&);
	QDataStream& operator>> (QDataStream&, Proxy&);
	QDataStream& operator<< (QDataStream&, const ReqTarget&);
	QDataStream& operator>> (QDataStream&, ReqTarget&);

	class ProxiesConfigWidget : public QWidget
	{
		Q_OBJECT

		Ui::ProxiesConfigWidget Ui_;
		QStandardItemModel *Model_;

		QList<Entry_t> Entries_;
	public:
		ProxiesConfigWidget (QWidget* = 0);

		QList<Proxy> FindMatching (const QString& reqHost, int reqPort,
				const QString& proto = QString ());
	private:
		void LoadSettings ();
		void SaveSettings () const;
	public slots:
		void accept ();
		void reject ();
	private slots:
		void on_AddProxyButton__released ();
	};
}
}
