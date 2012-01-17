//------------------------------------------------------------------------------------------------------------------------------------------
//    Copyright (c) 2011, Eugene Mamin <TheDZhon@gmail.com>
//    All rights reserved.
//
//    Redistribution and use in source and binary forms, with or without
//    modification, are permitted provided that the following conditions are met:
//        * Redistributions of source code must retain the above copyright
//        notice, this list of conditions and the following disclaimer.
//        * Redistributions in binary form must reproduce the above copyright
//        notice, this list of conditions and the following disclaimer in the
//        documentation and/or other materials provided with the distribution.
//        * Neither the name of the Prefix Increment nor the
//        names of its contributors may be used to endorse or promote products
//        derived from this software without specific prior written permission.
//
//    THIS SOFTWARE IS PROVIDED BY Eugene Mamin <TheDZhon@gmail.com> ''AS IS'' AND ANY
//    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
//    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//    DISCLAIMED. IN NO EVENT SHALL Eugene Mamin <TheDZhon@gmail.com> BE LIABLE FOR ANY
//    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
//    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
//    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
//    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------------------------------------------------------------------

#ifndef FAKEQWIDGETWINAPI_H__
#define FAKEQWIDGETWINAPI_H__

#if !defined(_WIN32)
#error You can't use this file in non-windows environment!
#endif

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <QWidget>
#include <QString>
#include <windows.h>

namespace LeechCraft 
{
namespace Liznoo
{
	class FakeQWidgetWinAPI : public QWidget
	{
		Q_OBJECT
	public:
		FakeQWidgetWinAPI(QWidget * parent = NULL);
	signals:
		void schemeChanged(QString schemeName);
		void powerSourceChanged(QString powerSource);
		void batteryStateChanged(int newPercentage);
	protected:
		virtual void handleSchemeChange(PPOWERBROADCAST_SETTING setting);
		virtual void handlePowerSourceChange(PPOWERBROADCAST_SETTING setting);
		virtual void handleBatteryStateChange(PPOWERBROADCAST_SETTING setting);

		virtual bool winEvent (MSG * message, long * result);
	};
} // namespace Liznoo
} // namespace Leechcraft

#endif // FAKEQWIDGETWINAPI_H__

