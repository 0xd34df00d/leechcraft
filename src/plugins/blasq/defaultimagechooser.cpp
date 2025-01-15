/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "defaultimagechooser.h"
#include <QDialog>
#include <QToolBar>
#include <QDialogButtonBox>
#include <interfaces/core/iiconthememanager.h>
#include <util/gui/geometry.h>
#include "photostab.h"
#include "interfaces/blasq/collection.h"

namespace LC
{
namespace Blasq
{
	DefaultImageChooser::DefaultImageChooser (AccountsManager *accMgr, const ICoreProxy_ptr& proxy, const QByteArray& accId)
	: AccMgr_ (accMgr)
	, Proxy_ (proxy)
	, Photos_ (new PhotosTab (accMgr, proxy))
	{
		auto dialog = new QDialog ();
		dialog->setWindowTitle (tr ("Choose an image to insert"));

		proxy->GetIconThemeManager ()->ManageWidget (dialog);

		auto lay = new QVBoxLayout ();
		dialog->setLayout (lay);

		auto toolbar = Photos_->GetToolBar();

		lay->addWidget (toolbar);
		lay->addWidget (Photos_);

		auto buttonBox = new QDialogButtonBox (QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		connect (buttonBox,
				SIGNAL (accepted ()),
				dialog,
				SLOT (accept ()));
		connect (buttonBox,
				SIGNAL (rejected ()),
				dialog,
				SLOT (reject ()));
		lay->addWidget (buttonBox);

		const auto& geom = Util::AvailableGeometry (QCursor::pos ());
		dialog->resize (geom.size () * 2 / 3);

		dialog->setAttribute (Qt::WA_DeleteOnClose);
		dialog->show ();

		connect (dialog,
				SIGNAL (accepted ()),
				this,
				SLOT (handleAccept ()));
		connect (dialog,
				SIGNAL (rejected ()),
				this,
				SLOT (handleReject ()));

		if (!accId.isEmpty ())
			Photos_->SelectAccount (accId);
	}

	QObject* DefaultImageChooser::GetQObject ()
	{
		return this;
	}

	RemoteImageInfos_t DefaultImageChooser::GetInfos () const
	{
		return Selected_;
	}

	void DefaultImageChooser::handleAccept ()
	{
		for (const auto& index : Photos_->GetSelectedImages ())
			Selected_.append ({
					index.data (CollectionRole::Original).toUrl (),
					index.data (CollectionRole::OriginalSize).toSize (),
					index.data (CollectionRole::MediumThumb).toUrl (),
					index.data (CollectionRole::MediumThumbSize).toSize (),
					index.data (CollectionRole::SmallThumb).toUrl (),
					index.data (CollectionRole::SmallThumbSize).toSize (),
					index.data (CollectionRole::Name).toString ()
				});

		emit ready ();

		delete Photos_;
		deleteLater ();
	}

	void DefaultImageChooser::handleReject ()
	{
		emit ready ();

		delete Photos_;
		deleteLater ();
	}
}
}
