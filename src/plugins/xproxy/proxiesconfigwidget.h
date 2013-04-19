/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
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
				const QString& proto = QString ()) const;
	private:
		void LoadSettings ();
		void SaveSettings () const;
		Entry_t EntryFromUI () const;
	public slots:
		void accept ();
		void reject ();
	private slots:
		void handleItemSelected (const QModelIndex&);
		void on_AddProxyButton__released ();
		void on_UpdateProxyButton__released ();
		void on_RemoveProxyButton__released ();
	};
}
}
