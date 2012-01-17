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

#ifndef PLATFORMWINAPI_H__
#define PLATFORMWINAPI_H__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
	#pragma once
#endif

#include <memory>
#include <windows.h>
#include "platformlayer.h"

namespace LeechCraft 
{
namespace Liznoo
{
	struct HPowerNotifyDeleter {
		void operator()(PHPOWERNOTIFY ptr) const
		{
			if(ptr) {
				if(*ptr) { UnregisterPowerSettingNotification(*ptr); }
				delete ptr;
			}
		}
	};

	class FakeQWidgetWinAPI;
	class PlatformWinAPI : public PlatformLayer
	{
		Q_OBJECT

		std::unique_ptr<HPOWERNOTIFY, HPowerNotifyDeleter> HPowerSchemeNotify_;
		std::unique_ptr<HPOWERNOTIFY, HPowerNotifyDeleter> HPowerSourceNotify_;
		std::unique_ptr<HPOWERNOTIFY, HPowerNotifyDeleter> HBatteryPowerNotify_;
		std::unique_ptr<FakeQWidgetWinAPI> FakeWidget_;
	public:
		PlatformWinAPI (QObject* = 0);
		virtual void Stop ();
	private slots:
		void handleSchemeChanged(QString schemeName);
		void handlePowerSourceChanged(QString powerSource);
		void handleBatteryStateChanged(int newPercentage);
	};
} // namespace Liznoo
} // namespace Leechcraft

#endif // PLATFORMWINAPI_H__

