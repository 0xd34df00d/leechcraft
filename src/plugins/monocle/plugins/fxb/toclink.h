/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/monocle/ilink.h>

namespace LC
{
namespace Monocle
{
namespace FXB
{
	class Document;

	class TOCLink : public QObject
				 , public ILink
	{
		Q_OBJECT

		Document *Doc_;
		int Page_;
	public:
		TOCLink (Document*, int);

		LinkType GetLinkType () const;
		QRectF GetArea () const;

		void Execute ();
	};
}
}
}
