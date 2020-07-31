/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/azoth/iprotocol.h>
#include <interfaces/azoth/imucprotocol.h>

namespace LC
{
namespace Azoth
{
class IProxyObject;

namespace Murm
{
	class VkAccount;
	class PhotoUrlStorage;

	class VkProtocol final : public QObject
						   , public IProtocol
						   , public IMUCProtocol
	{
		Q_OBJECT
		Q_INTERFACES (LC::Azoth::IProtocol LC::Azoth::IMUCProtocol)

		const ICoreProxy_ptr Proxy_;
		IProxyObject * const AzothProxy_;
		QObject * const Plugin_;

		QList<VkAccount*> Accounts_;

		PhotoUrlStorage * const PhotoUrlStorage_;
	public:
		VkProtocol (ICoreProxy_ptr, IProxyObject*, QObject*);
		~VkProtocol ();

		IProxyObject* GetAzothProxy () const;

		QObject* GetQObject ();
		ProtocolFeatures GetFeatures () const;
		QList<QObject*> GetRegisteredAccounts ();
		QObject* GetParentProtocolPlugin () const;

		QString GetProtocolName () const;
		QIcon GetProtocolIcon () const;
		QByteArray GetProtocolID () const;

		QList<QWidget*> GetAccountRegistrationWidgets (AccountAddOptions options);
		void RegisterAccount (const QString& name, const QList<QWidget*>& widgets);
		QWidget* GetMUCJoinWidget ();
		void RemoveAccount (QObject* account);

		PhotoUrlStorage* GetPhotoUrlStorage () const;
	private:
		void AddAccount (VkAccount*);
	private slots:
		void saveAccount (VkAccount*);
	signals:
		void accountAdded (QObject*);
		void accountRemoved (QObject*);
	};
}
}
}
