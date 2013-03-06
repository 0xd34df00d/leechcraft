/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_GLOOXACCOUNTCONFIGURATIONWIDGET_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_GLOOXACCOUNTCONFIGURATIONWIDGET_H
#include <QWidget>
#include <QXmppTransferManager.h>
#include <QXmppConfiguration.h>
#include "ui_glooxaccountconfigurationwidget.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	class GlooxAccountConfigurationWidget : public QWidget
	{
		Q_OBJECT

		Ui::GlooxAccountConfigurationWidget Ui_;

		QString Password_;
	public:
		GlooxAccountConfigurationWidget (QWidget* = 0);

		QString GetJID () const;
		void SetJID (const QString&);
		QString GetNick () const;
		void SetNick (const QString&);
		QString GetResource () const;
		void SetResource (const QString&);
		short GetPriority () const;
		void SetPriority (short);

		QString GetHost () const;
		void SetHost (const QString&);
		int GetPort () const;
		void SetPort (int);

		int GetKAInterval () const;
		void SetKAInterval (int);
		int GetKATimeout () const;
		void SetKATimeout (int);

		bool GetFileLogEnabled () const;
		void SetFileLogEnabled (bool);

		QXmppConfiguration::StreamSecurityMode GetTLSMode () const;
		void SetTLSMode (QXmppConfiguration::StreamSecurityMode);

		QXmppTransferJob::Methods GetFTMethods () const;
		void SetFTMethods (QXmppTransferJob::Methods);

		bool GetUseSOCKS5Proxy () const;
		void SetUseSOCKS5Proxy (bool);

		QString GetSOCKS5Proxy () const;
		void SetSOCKS5Proxy (const QString&);

		QString GetPassword () const;
	private slots:
		void on_UpdatePassword__released ();
	};
}
}
}

#endif
