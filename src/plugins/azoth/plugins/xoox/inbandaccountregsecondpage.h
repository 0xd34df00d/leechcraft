/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWizardPage>
#include <QXmppClient.h>
#include <interfaces/azoth/icanhavesslerrors.h>

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	class InBandAccountRegFirstPage;
	class RegFormHandlerWidget;

	class InBandAccountRegSecondPage : public QWizardPage
									 , public ICanHaveSslErrors
	{
		Q_OBJECT
		Q_INTERFACES (LC::Azoth::ICanHaveSslErrors)

		QXmppClient * const Client_;
		RegFormHandlerWidget *RegForm_;
		InBandAccountRegFirstPage *FirstPage_;

		bool SslAborted_ = false;
	public:
		InBandAccountRegSecondPage (InBandAccountRegFirstPage*, QWidget* = 0);

		void Register ();

		QString GetJID () const;
		QString GetPassword () const;

		QObject* GetQObject () override;

		bool isComplete () const override;
		void initializePage () override;
	private:
		void Reinitialize ();
	private slots:
		void handleConnected ();
		void handleClientError (QXmppClient::Error);
	signals:
		void sslErrors (const QList<QSslError>&,
				const ICanHaveSslErrors::ISslErrorsReaction_ptr&) override;
		void successfulReg ();
		void regError (const QString&);
	};
}
}
}
