/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#ifndef PLUGINS_AZOTH_MEDIADEVICEMANAGER_H
#define PLUGINS_AZOTH_MEDIADEVICEMANAGER_H
#include <QObject>
#include <QMap>
#include <QStringList>
#include <QGst/Global>
#include <QGst/PropertyProbe>
#include <interfaces/ihavesettings.h>

namespace LeechCraft
{
namespace Azoth
{
	class IMediaCall;
	class IODeviceSink;

	class MediaDeviceManager : public QObject
	{
		Q_OBJECT

		Util::XmlSettingsDialog_ptr XSD_;

	public:
		enum DeviceType
		{
			DTAudioIn,
			DTAudioOut
		};
	private:
		QMap<DeviceType, QGst::PropertyProbePtr> Type2Probe_;
		QMap<DeviceType, QStringList> Type2Devices_;

		class CallAudioHandler
		{
			boost::shared_ptr<IODeviceSink> IODeviceSink_;

			QGst::PipelinePtr OutPipe_;
			QGst::PipelinePtr InPipe_;
		public:
			CallAudioHandler (IMediaCall*);
		};
		typedef boost::shared_ptr<CallAudioHandler> CallAudioHandler_ptr;

		QMap<IMediaCall*, CallAudioHandler_ptr> Call2AudioHandler_;
	public:
		MediaDeviceManager (QObject* = 0);

		void SetXSD (Util::XmlSettingsDialog_ptr);

		void HandleCall (IMediaCall*);
	private:
		void FindDevices (DeviceType);
		void ProbeForDevices (const QGst::PropertyProbePtr&);
		void AddDevices (DeviceType, const QStringList& = QStringList ());
	};
}
}

#endif
