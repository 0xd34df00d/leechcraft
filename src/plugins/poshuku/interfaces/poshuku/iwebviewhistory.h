/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

namespace LC
{
namespace Poshuku
{
	class IWebViewHistory
	{
	protected:
		virtual ~IWebViewHistory () = default;
	public:
		virtual void Save (QDataStream& out) const = 0;
		virtual void Load (QDataStream& in) = 0;

		class IItem
		{
		protected:
			virtual ~IItem () = default;
		public:
			virtual bool IsValid () const = 0;

			virtual QString GetTitle () const = 0;

			virtual QUrl GetUrl () const = 0;

			virtual QIcon GetIcon () const = 0;

			virtual void Navigate () = 0;
		};

		using IItem_ptr = std::shared_ptr<IItem>;

		enum class Direction
		{
			Backward,
			Forward
		};

		virtual QList<IItem_ptr> GetItems (Direction dir, int maxItems) const = 0;
	};
}
}
