/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "captchamanager.h"
#include <algorithm>
#include <memory>
#include <QDialog>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QtDebug>
#include "xeps/xmppcaptchamanager.h"
#include "formbuilder.h"
#include "xeps/xmppbobmanager.h"

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	CaptchaManager::CaptchaManager (XMPPCaptchaManager& captchaMgr, XMPPBobManager& bobMgr, QObject *parent)
	: QObject (parent)
	, CaptchaManager_ (captchaMgr)
	, BobManager_ (bobMgr)
	{
		connect (&captchaMgr,
				&XMPPCaptchaManager::captchaFormReceived,
				this,
				&CaptchaManager::HandleCaptchaReceived);
	}

	void CaptchaManager::HandleCaptchaReceived (const QString& jid, const QXmppDataForm& dataForm)
	{
		auto builder = std::make_shared<FormBuilder> (jid, &BobManager_);

		auto dialog = new QDialog ();
		auto widget = builder->CreateForm (dataForm, dialog);
		dialog->setWindowTitle (widget->windowTitle ().isEmpty () ?
				tr ("Enter CAPTCHA") :
				widget->windowTitle ());
		dialog->setLayout (new QVBoxLayout ());
		dialog->layout ()->addWidget (widget);
		const auto box = new QDialogButtonBox (QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		dialog->layout ()->addWidget (box);
		dialog->setAttribute (Qt::WA_DeleteOnClose);

		connect (box,
				&QDialogButtonBox::accepted,
				dialog,
				&QDialog::accept);
		connect (box,
				&QDialogButtonBox::rejected,
				dialog,
				&QDialog::reject);

		connect (dialog,
				&QDialog::finished,
				this,
				[this, builder, jid] (int result)
				{
					if (result == QDialog::Accepted)
						CaptchaManager_.SendResponse (jid, builder->GetForm ());
				});

		dialog->show ();
	}
}
}
}
