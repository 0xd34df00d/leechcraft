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

#include "mediadevicemanager.h"
#include <cstring>
#include <QStringListModel>
#include <QGst/Init>
#include <QGst/ElementFactory>
#include <QGst/ChildProxy>
#include <QGst/PropertyProbe>
#include <QGst/Bin>
#include <QGst/Pipeline>
#include <QGlib/Connect>
#include <QGlib/Error>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "interfaces/imediacall.h"
#include "xmlsettingsmanager.h"
#include "iodevicesink.h"
#include "iodevicesrc.h"

namespace LeechCraft
{
namespace Azoth
{
	class CallAudioHandler
	{
		boost::shared_ptr<IODeviceSink> IODeviceSink_;
		boost::shared_ptr<IODeviceSrc> IODeviceSrc_;
		
		QGst::PipelinePtr OutPipe_;
		QGst::PipelinePtr InPipe_;
	public:
		CallAudioHandler (IMediaCall*);
	};

	CallAudioHandler::CallAudioHandler (IMediaCall *call)
	: IODeviceSink_ (new IODeviceSink (call->GetAudioDevice ()))
	, IODeviceSrc_ (new IODeviceSrc (call->GetAudioDevice ()))
	{
		QGst::BinPtr outAudioBin, inAudioBin;
		try
		{
			outAudioBin = QGst::Bin::fromDescription ("autoaudiosrc name=\"audiosrc\" ! audioconvert ! "
					"audioresample ! audiorate ! speexenc ! queue");
			inAudioBin = QGst::Bin::fromDescription ("autoaudiosink name=\"audiosink\" ! audioconvert ! "
					"audioresample ! audiorate ! speexenc ! queue");
		}
		catch (const QGlib::Error& error)
		{
			qWarning () << Q_FUNC_INFO
					<< "failed to create audio source bin:"
					<< error;
			return;
		}

		OutPipe_ = QGst::Pipeline::create ();
		OutPipe_->add (outAudioBin, IODeviceSink_->element ());
		outAudioBin->link (IODeviceSink_->element ());
		OutPipe_->setState (QGst::StatePlaying);
		
		InPipe_ = QGst::Pipeline::create ();
		InPipe_->add (IODeviceSrc_->element (), inAudioBin);
		inAudioBin->link (IODeviceSrc_->element ());
		InPipe_->setState (QGst::StatePlaying);
	}

	MediaDeviceManager::MediaDeviceManager (QObject *parent)
	: QObject (parent)
	{
		int argc = 1;
		const char* name = "leechcraft";
		char **argv = new char* [1];
		argv [0] = new char [std::strlen (name)];
		std::strcpy (argv [0], name);
		QGst::init (&argc, &argv);
	}

	void MediaDeviceManager::SetXSD (Util::XmlSettingsDialog_ptr xsd)
	{
		XSD_ = xsd;

		AddDevices (DTAudioIn);
		AddDevices (DTAudioOut);

		FindDevices (DTAudioIn);
		FindDevices (DTAudioOut);
	}

	namespace
	{
		QString GetElementName (MediaDeviceManager::DeviceType device)
		{
			switch (device)
			{
			case MediaDeviceManager::DTAudioIn:
				return "autoaudiosrc";
			case MediaDeviceManager::DTAudioOut:
				return "autoaudiosink";
			}
		}
	}

	void MediaDeviceManager::HandleCall (IMediaCall *call)
	{
		Call2AudioHandler_ [call] = CallAudioHandler_ptr (new CallAudioHandler (call));
	}

	void MediaDeviceManager::FindDevices (MediaDeviceManager::DeviceType device)
	{
		const QString& srcElementName = GetElementName (device);

		QGst::ElementPtr src = QGst::ElementFactory::make (srcElementName);
		if (!src)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to create element"
					<< srcElementName;
			return;
		}

		QGst::PropertyProbePtr propertyProbe;

		src->setState (QGst::StateReady);

		QGst::ChildProxyPtr childProxy = src.dynamicCast<QGst::ChildProxy> ();
		if (childProxy && childProxy->childrenCount () > 0)
			propertyProbe = childProxy->childByIndex (0).dynamicCast<QGst::PropertyProbe> ();

		src->setState (QGst::StateNull);

		if (propertyProbe && propertyProbe->propertySupportsProbe ("device"))
		{
			Type2Probe_ [device] = propertyProbe;

			ProbeForDevices (propertyProbe);

			QGlib::connect (propertyProbe,
					"probe-needed",
					this,
					&MediaDeviceManager::ProbeForDevices,
					QGlib::PassSender);
		}
		else
		{
			qWarning () << Q_FUNC_INFO
					<< "no property probe or property probe doesn't support \"device\""
					<< propertyProbe.isNull ();
			if (propertyProbe)
				Q_FOREACH (const QGlib::ParamSpecPtr& ptr, propertyProbe->properties ())
					qWarning () << ptr->name () << ptr->description ();
			AddDevices (device);
		}
	}

	void MediaDeviceManager::ProbeForDevices (const QGst::PropertyProbePtr& propertyProbe)
	{
		const DeviceType type = Type2Probe_.key (propertyProbe);
		Type2Devices_ [type].clear ();

		QStringList names;

		Q_FOREACH(const QGlib::Value& device,
				propertyProbe->probeAndGetValues ("device"))
		{
			propertyProbe->setProperty ("device", device);
			const QString& deviceName = propertyProbe->
					property ("device-name").toString();

			names << QString ("%1 (%2)").arg (deviceName, device.toString ());
			Type2Devices_ [type] << device.toString ();
		}

		AddDevices (type, names);
	}

	void MediaDeviceManager::AddDevices (MediaDeviceManager::DeviceType device,
			const QStringList& others)
	{
		QString userVisible;
		QByteArray propName;

		switch (device)
		{
		case DTAudioIn:
			userVisible = tr ("Default input device");
			propName = "InputAudioDevice";
			break;
		case DTAudioOut:
			userVisible = tr ("Default output device");
			propName = "OutputAudioDevice";
			break;
		}

		QStringList audios (userVisible);
		audios << others;
		XSD_->SetDataSource (propName, new QStringListModel (audios));
	}
}
}
