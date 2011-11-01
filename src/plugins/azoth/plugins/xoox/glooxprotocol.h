/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_GLOOXPROTOCOL_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_GLOOXPROTOCOL_H
#include <QObject>
#include <interfaces/iprotocol.h>
#include <interfaces/iurihandler.h>

namespace LeechCraft
{
struct Entity;

namespace Azoth
{
namespace Xoox
{
	class GlooxAccount;

	class GlooxProtocol : public QObject
						, public IProtocol
						, public IURIHandler
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::Azoth::IProtocol LeechCraft::Azoth::IURIHandler);

		QObject *ParentProtocolPlugin_;
		QList<GlooxAccount*> Accounts_;
		QObject *ProxyObject_;
	public:
		GlooxProtocol (QObject*);
		virtual ~GlooxProtocol ();

		void Prepare ();

		QObject* GetProxyObject () const;
		void SetProxyObject (QObject*);

		QObject* GetObject ();
		ProtocolFeatures GetFeatures () const;
		QList<QObject*> GetRegisteredAccounts ();
		QObject* GetParentProtocolPlugin () const;
		QString GetProtocolName () const;
		QIcon GetProtocolIcon () const;
		QByteArray GetProtocolID () const;
		QList<QWidget*> GetAccountRegistrationWidgets (AccountAddOptions);
		void RegisterAccount (const QString&, const QList<QWidget*>&);
		QWidget* GetMUCJoinWidget ();
		void RemoveAccount (QObject*);

		bool SupportsURI (const QUrl&) const;
		void HandleURI (const QUrl&, QObject*);
	private:
		void RestoreAccounts ();
	private slots:
		void saveAccounts () const;
	signals:
		void accountAdded (QObject*);
		void accountRemoved (QObject*);
	};
}
}
}

#endif
