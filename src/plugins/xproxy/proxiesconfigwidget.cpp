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

#include "proxiesconfigwidget.h"
#include <QStandardItemModel>
#include <QSettings>

Q_DECLARE_METATYPE (QList<LeechCraft::XProxy::Entry_t>);

namespace LeechCraft
{
namespace XProxy
{
	Proxy::operator QNetworkProxy () const
	{
		return QNetworkProxy (Type_, Host_, Port_, User_, Pass_);
	}

	namespace
	{
		QString ProxyType2Str (QNetworkProxy::ProxyType type)
		{
			switch (type)
			{
			case QNetworkProxy::ProxyType::Socks5Proxy:
				return "SOCKS5";
				break;
			case QNetworkProxy::ProxyType::HttpProxy:
				return "HTTP";
				break;
			case QNetworkProxy::ProxyType::HttpCachingProxy:
				return ProxiesConfigWidget::tr ("caching HTTP");
				break;
			case QNetworkProxy::ProxyType::FtpCachingProxy:
				return ProxiesConfigWidget::tr ("caching FTP");
				break;
			default:
				return ProxiesConfigWidget::tr ("other type");
				break;
			}
		}

		QList<QStandardItem*> Entry2Row (const Entry_t& entry)
		{
			QList<QStandardItem*> row;

			const auto& req = entry.first;
			if (req.Protocols_.isEmpty ())
				row << new QStandardItem (ProxiesConfigWidget::tr ("any"));
			else
				row << new QStandardItem (req.Protocols_.join ("; "));
			const QString& targetStr = req.Host_.pattern () +
					":" +
					(req.Port_ > 0 ?
							QString::number (req.Port_) :
							ProxiesConfigWidget::tr ("any"));
			row << new QStandardItem (targetStr);

			const auto& proxy = entry.second;
			row << new QStandardItem (ProxyType2Str (proxy.Type_));
			row << new QStandardItem (proxy.Host_ + ":" + QString::number (proxy.Port_));
			row << new QStandardItem (proxy.User_);

			return row;
		}
	}

	ProxiesConfigWidget::ProxiesConfigWidget (QWidget *parent)
	: QWidget (parent)
	, Model_ (new QStandardItemModel (this))
	{
		Ui_.setupUi (this);
		Ui_.ProxiesList_->setModel (Model_);

		reject ();

		auto protoModel = Ui_.TargetProto_->model ();
		for (int i = 0; i < protoModel->rowCount (); ++i)
		{
			const auto& idx = protoModel->index (i, 0);
			protoModel->setData (idx, Qt::Unchecked, Qt::CheckStateRole);
		}
	}

	QList<Proxy> ProxiesConfigWidget::FindMatching (const QString& reqHost, int reqPort, const QString& proto)
	{
		QList<Proxy> result;
		Q_FOREACH (const auto& pair, Entries_)
		{
			const auto& target = pair.first;
			if (target.Port_ && reqPort > 0 && target.Port_ != reqPort)
				continue;

			if (!target.Protocols_.isEmpty () && !target.Protocols_.contains (proto))
				continue;

			if (!target.Host_.exactMatch (reqHost))
				continue;

			result << pair.second;
		}
		return result;
	}

	void ProxiesConfigWidget::LoadSettings ()
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_XProxy");
		settings.beginGroup ("SavedProxies");
		Entries_ = settings.value ("Entries").value<decltype (Entries_)> ();
		settings.endGroup ();

		Q_FOREACH (const auto& entry, Entries_)
			Model_->appendRow (Entry2Row (entry));
	}

	void ProxiesConfigWidget::SaveSettings () const
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_XProxy");
		settings.beginGroup ("SavedProxies");
		settings.setValue ("Entries", QVariant::fromValue<decltype (Entries_)> (Entries_));
		settings.endGroup ();
	}

	void ProxiesConfigWidget::accept ()
	{
		SaveSettings ();
	}

	void ProxiesConfigWidget::reject ()
	{
		Model_->clear ();

		QStringList labels;
		labels << tr ("Protocols")
				<< tr ("Target")
				<< tr ("Proxy type")
				<< tr ("Proxy target")
				<< tr ("User");
		Model_->setHorizontalHeaderLabels (labels);

		LoadSettings ();
	}

	void ProxiesConfigWidget::on_AddProxyButton__released ()
	{
		QString rxPat = Ui_.TargetHost_->text ();
		if (!rxPat.contains ("*") && !rxPat.contains ("^") && !rxPat.contains ("$"))
		{
			rxPat.prepend (".*");
			rxPat.append (".*");
		}

		QStringList protos;
		auto protoModel = Ui_.TargetProto_->model ();
		for (int i = 0; i < protoModel->rowCount (); ++i)
		{
			const auto& idx = protoModel->index (i, 0);
			if (idx.data (Qt::CheckStateRole).toInt () == Qt::Checked)
				protos << idx.data (Qt::DisplayRole).toString ();
		}
		protos.removeAll (QString ());

		ReqTarget targ =
		{
			QRegExp (rxPat, Qt::CaseInsensitive),
			Ui_.TargetPort_->value (),
			protos
		};

		QNetworkProxy::ProxyType type = QNetworkProxy::ProxyType::NoProxy;
		switch (Ui_.ProxyType_->currentIndex ())
		{
			case 0:
				type = QNetworkProxy::ProxyType::Socks5Proxy;
				break;
			case 1:
				type = QNetworkProxy::ProxyType::HttpProxy;
				break;
			case 2:
				type = QNetworkProxy::ProxyType::HttpCachingProxy;
				break;
			case 3:
				type = QNetworkProxy::ProxyType::FtpCachingProxy;
				break;
		}
		Proxy proxy =
		{
			type,
			Ui_.ProxyHost_->text (),
			Ui_.ProxyPort_->value (),
			Ui_.ProxyUser_->text (),
			Ui_.ProxyPassword_->text ()
		};

		const auto& entry = qMakePair (targ, proxy);
		Entries_ << entry;
		Model_->appendRow (Entry2Row (entry));

		SaveSettings ();
	}

	QDataStream& operator<< (QDataStream& out, const Proxy& p)
	{
		out << static_cast<quint8> (1);
		out << static_cast<qint8> (p.Type_)
			<< p.Host_
			<< p.Port_
			<< p.User_
			<< p.Pass_;
		return out;
	}

	QDataStream& operator>> (QDataStream& in, Proxy& p)
	{
		quint8 ver = 0;
		in >> ver;
		if (ver != 1)
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown version"
					<< ver;
			return in;
		}

		qint8 type = 0;
		in >> type
			>> p.Host_
			>> p.Port_
			>> p.User_
			>> p.Pass_;
		p.Type_ = static_cast<QNetworkProxy::ProxyType> (type);

		return in;
	}

	QDataStream& operator<< (QDataStream& out, const ReqTarget& t)
	{
		out << static_cast<quint8> (1);
		out << t.Host_
			<< t.Port_
			<< t.Protocols_;
		return out;
	}

	QDataStream& operator>> (QDataStream& in, ReqTarget& t)
	{
		quint8 ver = 0;
		in >> ver;
		if (ver != 1)
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown version"
					<< ver;
			return in;
		}

		in >> t.Host_
			>> t.Port_
			>> t.Protocols_;
		return in;
	}
}
}
