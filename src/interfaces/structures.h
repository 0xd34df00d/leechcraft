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
		NoParameters = 0,
		Autostart = 1,
		DoNotSaveInHistory = 2,
		FromClipboard = 4,
		FromCommonDialog = 8,
		FromAutomatic = 16,
		DoNotNotifyUser = 32,
		FromAnother = 64,
		Internal = 128
	};

	Q_DECLARE_FLAGS (TaskParameters, TaskParameter);
};

Q_DECLARE_OPERATORS_FOR_FLAGS (LeechCraft::TaskParameters);

#endif

