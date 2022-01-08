/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "accountaddwizard.h"
#include <QRegExpValidator>
#include <QtDebug>
#include "ui_accountaddwizardemailpage.h"
#include "ui_accountaddwizardtemplatepage.h"
#include "accountconfig.h"

namespace LC
{
namespace Snails
{
	namespace
	{
		class EmailPage : public QWizardPage
		{
			Q_DECLARE_TR_FUNCTIONS (EmailPage)

			Ui::AccountAddWizardEmailPage Ui_;
		public:
			EmailPage (QWidget *parent = nullptr)
			: QWizardPage { parent }
			{
				Ui_.setupUi (this);

				setTitle (tr ("Basic account information"));

				const QRegExp emailRegexp { "*@*", Qt::CaseInsensitive, QRegExp::Wildcard };
				Ui_.Email_->setValidator (new QRegExpValidator { emailRegexp });

				registerField ("accName*", Ui_.AccName_);
				registerField ("email*", Ui_.Email_);
				registerField ("name", Ui_.Name_);
			}
		};

		enum class KnownService
		{
			Unknown,
			GMail
		};

		constexpr auto KnownServicesCount = static_cast<uint16_t> (KnownService::GMail) + 1;

		template<typename F>
		void EnumServices (F&& f)
		{
			for (uint16_t i = 0; i < KnownServicesCount; ++i)
				if constexpr (std::is_invocable_v<std::decay_t<F>, KnownService, uint16_t>)
					std::invoke (f, static_cast<KnownService> (i), i);
				else
					std::invoke (f, static_cast<KnownService> (i));
		}

		QString GetServiceName (KnownService service)
		{
			switch (service)
			{
			case KnownService::Unknown:
				return "<custom service>";
			case KnownService::GMail:
				return "GMail";
			}
		}

		KnownService GuessService (QString email)
		{
			email = std::move (email).trimmed ();

			if (email.endsWith ("gmail.com"))
				return KnownService::GMail;

			return KnownService::Unknown;
		}

		constexpr uint16_t SvcPagesOffset = 100;

		class TemplatePage : public QWizardPage
		{
			Q_DECLARE_TR_FUNCTIONS (TemplatePage)

			Ui::AccountAddWizardTemplatePage Ui_;
		public:
			TemplatePage (QWidget *parent = nullptr)
			: QWizardPage { parent }
			{
				Ui_.setupUi (this);

				setTitle (tr ("Choose a service template"));

				EnumServices ([this] (KnownService svc) { Ui_.MailTemplate_->addItem (GetServiceName (svc)); });
			}

			void initializePage () override
			{
				QWizardPage::initializePage ();

				const auto& email = field ("email").toString ();
				const auto service = GuessService (email);
				Ui_.MailTemplate_->setCurrentIndex (static_cast<int> (service));
			}

			int nextId () const override
			{
				return SvcPagesOffset + Ui_.MailTemplate_->currentIndex ();
			}
		};

		class IConfigGenerator
		{
		public:
			virtual ~IConfigGenerator () = default;

			virtual AccountConfig GetConfig () const = 0;
		};

		class CommonServicePage : public QWizardPage
								, public IConfigGenerator
		{
			Q_DECLARE_TR_FUNCTIONS (CommonServicePage)
		public:
			CommonServicePage (QWizardPage *parent = nullptr)
			: QWizardPage { parent }
			{
				setTitle ("Account configuration");

				auto lay = new QFormLayout;
				setLayout (lay);

				auto label = new QLabel { tr ("That's it for the wizard! "
						"The account configuration dialog will now open "
						"where you will be able to further configure the account.") };

				lay->addWidget (label);
			}

			int nextId () const override
			{
				return -1;
			}

			AccountConfig GetConfig () const override
			{
				const auto& email = field ("email").toString ().trimmed ();
				const auto& domain = email.section ('@', -1, -1);

				return
				{
					.AccName_ = field ("accName").toString (),
					.UserName_ = field ("name").toString (),
					.UserEmail_ = field ("email").toString (),

					.Login_ = email,
					.UseSASL_ = false,
					.SASLRequired_ = false,

					.InSecurity_ = AccountConfig::SecurityType::TLS,
					.InSecurityRequired_ = true,
					.OutSecurity_ = AccountConfig::SecurityType::TLS,
					.OutSecurityRequired_ = true,

					.InHost_ = "imap." + domain,
					.InPort_ = 993,
					.OutHost_ = "smtp." + domain,
					.OutPort_ = 465,
					.OutLogin_ = {},
					.OutType_ = AccountConfig::OutType::SMTP
				};
			}
		};

		class GMailServicePage : public QWizardPage
							   , public IConfigGenerator
		{
			QLineEdit * const Login_;
		public:
			GMailServicePage (QWizardPage *parent = nullptr)
			: QWizardPage { parent }
			, Login_ { new QLineEdit }
			{
				setTitle ("GMail account configuration");

				auto lay = new QFormLayout;
				setLayout (lay);

				lay->addRow (new QLabel { tr ("Login:") }, Login_);

				registerField ("gmailLogin*", Login_);
			}

			void initializePage () override
			{
				const auto& email = field ("email").toString ().trimmed ();
				Login_->setText (email);
			}

			int nextId () const override
			{
				return -1;
			}

			AccountConfig GetConfig () const override
			{
				return
				{
					.AccName_ = field ("accName").toString (),
					.UserName_ = field ("name").toString (),
					.UserEmail_ = field ("email").toString (),

					.Login_ = Login_->text (),
					.UseSASL_ = false,
					.SASLRequired_ = false,

					.InSecurity_ = AccountConfig::SecurityType::SSL,
					.InSecurityRequired_ = true,
					.OutSecurity_ = AccountConfig::SecurityType::SSL,
					.OutSecurityRequired_ = true,
					.SMTPNeedsAuth_ = true,

					.InHost_ = "imap.gmail.com",
					.InPort_ = 993,
					.OutHost_ = "smtp.gmail.com",
					.OutPort_ = 465,
					.OutLogin_ = {},

					.OutType_ = AccountConfig::OutType::SMTP
				};
			}
		};

		QWizardPage* MakeServicePage (KnownService service)
		{
			switch (service)
			{
			case KnownService::Unknown:
				return new CommonServicePage;
			case KnownService::GMail:
				return new GMailServicePage;
			}
		}
	}

	AccountAddWizard::AccountAddWizard (QWidget *parent)
	: QWizard { parent }
	{
		addPage (new EmailPage);
		addPage (new TemplatePage);

		EnumServices ([this] (KnownService svc, uint16_t idx) { setPage (SvcPagesOffset + idx, MakeServicePage (svc)); });

		setWindowTitle (tr ("Account creation wizard"));
	}

	AccountConfig AccountAddWizard::GetConfig () const
	{
		const auto& pgs = visitedIds ();
		if (pgs.isEmpty ())
			return {};

		const auto lastPage = page (pgs.last ());
		const auto icg = dynamic_cast<IConfigGenerator*> (lastPage);
		if (!icg)
		{
			qWarning () << Q_FUNC_INFO
					<< "last page is not a config generator:"
					<< lastPage;
			return {};
		}

		return icg->GetConfig ();
	}
}
}
