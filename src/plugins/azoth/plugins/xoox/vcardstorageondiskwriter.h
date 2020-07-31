/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <util/threads/workerthreadbase.h>
#include "vcardstorageondisk.h"

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	class VCardStorageOnDisk;

	class VCardStorageOnDiskWriter final : public Util::WorkerThreadBase
	{
		std::unique_ptr<VCardStorageOnDisk> Storage_;
	public:
		using Util::WorkerThreadBase::WorkerThreadBase;

		QFuture<void> SetVCard (const QString&, const QString&);
		QFuture<void> SetVCardPhotoHash (const QString&, const QByteArray&);
	protected:
		void Initialize () override;
		void Cleanup () override;
	};
}
}
}
