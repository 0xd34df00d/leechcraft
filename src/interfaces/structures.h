#ifndef STRUCTURES_H
#define STRUCTURES_H

class QMenu;

struct DirectDownloadParams
{
    QString Resource_;
    QString Location_;
};

struct MainWindowExternals
{
    QMenu *RootMenu_;
};

namespace LeechCraft
{
	enum TaskParameter
	{
		NoParameters = 0x0,
		Autostart = 0x1,
		DoNotSaveInHistory = 0x2,
		FromClipboard = 0x4,
		FromCommonDialog = 0x8,
		FromAutomatic = 0x16,
		DoNotNotifyUser = 0x32
	};

	Q_DECLARE_FLAGS (TaskParameters, TaskParameter);
};

Q_DECLARE_OPERATORS_FOR_FLAGS (LeechCraft::TaskParameters);

#endif

