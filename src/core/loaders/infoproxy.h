/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/iinfo.h>

class QDBusInterface;

namespace LC
{
namespace Loaders
{
	class InfoProxy : public QObject
					, public IInfo
	{
		Q_OBJECT
		Q_INTERFACES (IInfo)

		const QString Service_;
		std::shared_ptr<QDBusInterface> IFace_;

		std::shared_ptr<QDBusInterface> Info_;
	public:
		explicit InfoProxy (const QString& service);

		void SetProxy (ICoreProxy_ptr proxy);
		void Init (ICoreProxy_ptr proxy);
		void SecondInit ();
		void Release ();
		QByteArray GetUniqueID () const;
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;
	};
}
}
