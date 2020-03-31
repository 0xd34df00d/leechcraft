/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 **********************************************************************/

#include "defaultimagechooser.h"
#include <QDialog>
#include <QToolBar>
#include <QDialogButtonBox>
#include <QDesktopWidget>
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
