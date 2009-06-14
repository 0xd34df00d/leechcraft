#ifndef PLUGINS_BITTORRENT_FILEINFO_H
#define PLUGINS_BITTORRENT_FILEINFO_H
#include <boost/filesystem/path.hpp>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace BitTorrent
		{
			struct FileInfo
			{
				boost::filesystem::path Path_;
				quint64 Size_;
				int Priority_;
				float Progress_;
			};
		};
	};
};

#endif

