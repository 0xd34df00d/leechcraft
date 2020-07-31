/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QtPlugin>

template<typename>
class QList;

class QSslError;

namespace LC
{
namespace Azoth
{
	class ICanHaveSslErrors
	{
	protected:
		virtual ~ICanHaveSslErrors () = default;
	public:
		class ISslErrorsReaction
		{
		protected:
			virtual ~ISslErrorsReaction () = default;
		public:
			virtual void Ignore () = 0;
			virtual void Abort () = 0;
		};

		using ISslErrorsReaction_ptr = std::shared_ptr<ISslErrorsReaction>;

		virtual QObject* GetQObject () = 0;
	protected:
		virtual void sslErrors (const QList<QSslError>&, const ISslErrorsReaction_ptr&) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::Azoth::ICanHaveSslErrors,
		"org.LeechCraft.Azoth.ICanHaveSslErrors/1.0")
