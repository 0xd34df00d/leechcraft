TEMPLATE = subdirs
CONFIG += release ordered
SUBDIRS += trunk/exceptions \
		   trunk/plugininterface \
		   trunk/settingsdialog \
		   trunk \
		   trunk/plugins/http \
		   trunk/plugins/updater \
		   trunk/plugins/torrent \
		   trunk/plugins/batcher \
		   trunk/plugins/cron
