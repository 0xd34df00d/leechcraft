/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_INBANDACCOUNTREGTHIRDPAGE_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_INBANDACCOUNTREGTHIRDPAGE_H
#include <QWizardPage>

class QLabel;

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	class InBandAccountRegSecondPage;
	class GlooxAccountConfigurationWidget;

	class InBandAccountRegThirdPage : public QWizardPage
	{
		Q_OBJECT

		InBandAccountRegSecondPage *SecondPage_;
		GlooxAccountConfigurationWidget *ConfWidget_;
		QLabel *StateLabel_;

		enum RegState
		{
			RSIdle,
			RSAwaitingResult,
			RSSuccess,
			RSError
		} RegState_;
	public:
		InBandAccountRegThirdPage (InBandAccountRegSecondPage*, QWidget* = 0);

		void SetConfWidget (GlooxAccountConfigurationWidget*);

		bool isComplete () const;
		void initializePage ();
	private:
		void SetState (RegState);
	private slots:
		void handleSuccessfulReg ();
		void handleRegError (const QString&);
	};
}
}
}

#endif
