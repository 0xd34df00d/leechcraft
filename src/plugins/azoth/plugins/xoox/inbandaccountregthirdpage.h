/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_INBANDACCOUNTREGTHIRDPAGE_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_INBANDACCOUNTREGTHIRDPAGE_H
#include <QWizardPage>

class QLabel;

namespace LeechCraft
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
