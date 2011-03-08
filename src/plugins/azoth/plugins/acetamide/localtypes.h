#ifndef PLUGINS_AZOTH_PLUGINS_ACETAMIDE_LOCALTYPES_H
#define PLUGINS_AZOTH_PLUGINS_ACETAMIDE_LOCALTYPES_H

#include <QString>
#include "channelclentry.h"
#include <boost/graph/graph_concepts.hpp>


namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	struct ServerOptions
	{
		QString NetworkName_;
		QString ServerName_;
		QString ServerEncoding_;
		QString ServerPassword_;
		QStringList ServerNicknames_;
		QString ServerRealName_;
		int ServerPort_;
		bool SSL_;
	};

	struct ChannelOptions
	{
		QString ServerName_;
		QString ChannelName_;
		QString ChannelPassword_;
		QString ChannelNickname_;
	};
	
	enum ConnectionState
	{
		Connected,
		InProcess,
		NotConnected
	};
};
};
};


#endif // PLUGINS_AZOTH_PLUGINS_ACETAMIDE_LOCALTYPES_H
