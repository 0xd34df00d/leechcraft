/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "macosbackend.h"
#include <limits>
#include <QtConcurrent>
#include <QCoreApplication>
#include <QtDebug>
#include <IOKit/IOKitLib.h>

namespace LC::HotSensors
{
	namespace
	{
		const QString DataTypeSp78 = "sp78";

		const int CmdReadBytes = 5;
		const int CmdReadKeyInfo = 9;

		const int KernelIndexSmc = 2;

		typedef char UInt32Char_t [5];
		typedef char SMCBytes_t [32];

		struct SMCVal_t
		{
			UInt32Char_t Key_;
			UInt32 DataSize_;
			UInt32Char_t DataType_;
			SMCBytes_t Bytes_;
		};

		struct SMCKeyData_vers_t
		{
			char Major_;
			char Minor_;
			char Build_;
			char Reserved_ [1];
			UInt16 Release_;
		};

		struct SMCKeyData_pLimitData_t
		{
			UInt16 Version_;
			UInt16 Length_;
			UInt32 CpuPLimit_;
			UInt32 GpuPLimit_;
			UInt32 MemPLimit_;
		};

		struct SMCKeyData_keyInfo_t
		{
			UInt32 DataSize_;
			UInt32 DataType_;
			char DataAttributes_;
		} ;

		struct SMCKeyData_t
		{
			UInt32 Key_;
			SMCKeyData_vers_t Vers_;
			SMCKeyData_pLimitData_t PLimitData_;
			SMCKeyData_keyInfo_t KeyInfo_;
			char Result_;
			char Status_;
			char Data8_;
			UInt32 Data32_;
			SMCBytes_t Bytes_;
		};

		class SMC
		{
			bool Valid_ = false;
			io_connect_t Conn_;
		public:
			SMC ();
			~SMC ();

			SMC (const SMC&) = delete;
			SMC& operator= (const SMC&) = delete;

			double GetTemp (const char *key);
		private:
			SMCVal_t ReadKey (const char *key);
		};

		SMC::SMC ()
		{
			mach_port_t masterPort;
			auto result = IOMasterPort (MACH_PORT_NULL, &masterPort);

			if (result != kIOReturnSuccess)
				return;

			auto matchingDict = IOServiceMatching ("AppleSMC");

			io_iterator_t iterator;
			result = IOServiceGetMatchingServices (masterPort, matchingDict, &iterator);
			if (result != kIOReturnSuccess)
			{
				qWarning () << Q_FUNC_INFO
						<< "no matching services:"
						<< result;
				return;
			}

			const auto device = IOIteratorNext (iterator);
			IOObjectRelease (iterator);

			if (!device)
			{
				qWarning () << Q_FUNC_INFO
						<< "no SMC found";
				return;
			}

			result = IOServiceOpen (device, mach_task_self (), 0, &Conn_);
			IOObjectRelease (device);
			if (result != kIOReturnSuccess)
			{
				qWarning () << Q_FUNC_INFO
						<< "cannot open service:"
						<< result;
				return;
			}

			Valid_ = true;
		}

		SMC::~SMC ()
		{
			IOServiceClose (Conn_);
		}

		double SMC::GetTemp (const char *key)
		{
			try
			{
				const auto val = ReadKey (key);
				if (!val.DataSize_)
					return 0;

				if (val.DataType_ != DataTypeSp78)
					return 0;

				const int intVal = (val.Bytes_ [0] * 256 + val.Bytes_ [1]) >> 2;
				return intVal / 64.0;
			}
			catch (const std::exception& e)
			{
				qWarning () << Q_FUNC_INFO
						<< e.what ();
				return 0;
			}
		}

		UInt32 HexStrToULong (const char *str, size_t size)
		{
			UInt32 result = 0;
			for (size_t i = 0; i < size; ++i)
				result += str [i] << (size - i - 1) * 8;
			return result;
		}

		void ULongToStr (char *str, UInt32 val)
		{
			str [0] = 0;
			std::sprintf (str,
					"%c%c%c%c",
					static_cast<unsigned int> (val) >> 24,
					static_cast<unsigned int> (val) >> 16,
					static_cast<unsigned int> (val) >> 8,
					static_cast<unsigned int> (val));
		}

		SMCVal_t SMC::ReadKey (const char *srcKey)
		{
			UInt32Char_t key;
			strncpy (key, srcKey, sizeof (key) / sizeof (key [0]));

			SMCKeyData_t inputStruct, outputStruct;
			memset (&inputStruct, 0, sizeof (SMCKeyData_t));
			memset (&outputStruct, 0, sizeof (SMCKeyData_t));

			inputStruct.Key_ = HexStrToULong (srcKey, std::strlen (srcKey));
			inputStruct.Data8_ = CmdReadKeyInfo;

			SMCVal_t result;
			memset (&result, 0, sizeof (result));
			strncpy (result.Key_, srcKey, sizeof (key) / sizeof (key [0]));

			auto outputSize = sizeof (outputStruct);
			if (IOConnectCallStructMethod (Conn_, KernelIndexSmc,
						&inputStruct, sizeof (inputStruct),
						&outputStruct, &outputSize) != kIOReturnSuccess)
				throw std::runtime_error ("Cannot call struct method");

			result.DataSize_ = outputStruct.KeyInfo_.DataSize_;
			ULongToStr (result.DataType_, outputStruct.KeyInfo_.DataType_);
			inputStruct.KeyInfo_.DataSize_ = result.DataSize_;
			inputStruct.Data8_ = CmdReadBytes;

			outputSize = sizeof (outputStruct);
			if (IOConnectCallStructMethod (Conn_, KernelIndexSmc,
						&inputStruct, sizeof (inputStruct),
						&outputStruct, &outputSize) != kIOReturnSuccess)
				throw std::runtime_error ("Cannot call struct method");

			memcpy (result.Bytes_, outputStruct.Bytes_, sizeof (outputStruct.Bytes_));

			return result;
		}

		void EnumerateSensors ()
		{
			QtConcurrent::run ([]
					{
						SMC smc;

						std::vector<char> allSyms;
						for (char c = 'A'; c <= 'Z'; ++c)
							allSyms.push_back (c);
						for (char c = 'a'; c <= 'z'; ++c)
							allSyms.push_back (c);
						for (char c = '0'; c <= '9'; ++c)
							allSyms.push_back (c);

						for (char c : allSyms)
							for (char n : allSyms)
								for (char s : allSyms)
								{
									char sensor [] = { 'T', c, n, s, '\0' };
									const auto temp = smc.GetTemp (sensor);
									if (temp > std::numeric_limits<double>::epsilon ())
										qDebug () << "got reading" << sensor << ":" << temp;
								}
					});
		}
	}

	MacOsBackend::MacOsBackend (QObject *parent)
	: Backend { parent }
	{
		if (QCoreApplication::arguments ().contains ("--enumerateSensors"))
			EnumerateSensors ();
	}

	void MacOsBackend::update ()
	{
		SMC smc;

		Readings_t readings;

		auto append = [&smc, &readings] (const QString& name, const char *sensor)
		{
			const auto temp = smc.GetTemp (sensor);
			if (temp > std::numeric_limits<double>::epsilon ())
				readings.push_back ({ name, smc.GetTemp (sensor), 105, 120 });
		};
		append ("CPU", "TC0P");
		append ("CPU diode", "TC0D");
		append ("GPU", "TG0P");
		append ("GPU diode", "TG0D");
		append ("Power supply proximity", "Tp0P");
		append ("Palm rest", "Ts0P");
		append ("Battery TS_MAX", "TB0T");
		append ("Battery 1", "TB1T");
		append ("Battery 2", "TB2T");
		append ("Memory proximity", "Ts0S");
		append ("PCH die", "TPCD");
		append ("Heat pipe 1", "Th1H");
		append ("Heat pipe 2", "Th2H");
		append ("PCH die", "TPCD");

		/** More at:
		 *
		 * https://github.com/jedda/OSX-Monitoring-Tools/blob/master/check_osx_smc/known-registers.md
		 * http://superuser.com/questions/553197/interpreting-sensor-names
		 * https://discussions.apple.com/thread/4838014?tstart=0
		 * http://www.parhelia.ch/blog/statics/k3_keys.html
		 * http://jbot-42.github.io/Articles/smc.html
		 */

		emit gotReadings (readings);
	}
}
