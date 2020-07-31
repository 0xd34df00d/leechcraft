/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/azoth/itransfermanager.h>

namespace LC
{
namespace Azoth
{
namespace Murm
{
	class VkAccount;

	class TransferManager : public QObject
						  , public ITransferManager
	{
		Q_OBJECT
		Q_INTERFACES (LC::Azoth::ITransferManager)

		VkAccount * const Acc_;
	public:
		TransferManager (VkAccount*);

		bool IsAvailable () const override;
		QObject* SendFile (const QString&, const QString&, const QString&, const QString&) override;
	signals:
		void fileOffered (QObject*) override;
	};
}
}
}
