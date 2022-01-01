/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include "qt5compat.h"
#include <poppler-qt5.h>
#include <interfaces/monocle/ilink.h>

namespace LC
{
namespace Monocle
{
namespace PDF
{
	class Document;

	class Link : public QObject
			   , public ILink
	{
		Q_INTERFACES (LC::Monocle::ILink)

		Document * const Doc_;
		std::shared_ptr<Poppler::Link> Link_;
	public:
		explicit Link (Document*, Poppler::Link*);
		explicit Link (Document*, Poppler::Link*, const std::shared_ptr<void>&);

		LinkType GetLinkType () const override;
		QRectF GetArea () const override;

		void Execute () override;
	private:
		void ExecutePageLink ();
		void ExecuteCommandLink ();
	};

	class TOCLink : public QObject
				  , public ILink
				  , public IPageLink
	{
		Q_INTERFACES (LC::Monocle::ILink LC::Monocle::IPageLink)

		Document * const Doc_;
		std::unique_ptr<Poppler::LinkDestination> Dest_;
	public:
		TOCLink (Document*, std::unique_ptr<Poppler::LinkDestination>);

		LinkType GetLinkType () const override;
		QRectF GetArea () const override;

		void Execute () override;

		QString GetDocumentFilename () const override;
		int GetPageNumber () const override;

		double NewX () const override;
		double NewY () const override;
		double NewZoom () const override;
	};
}
}
}
