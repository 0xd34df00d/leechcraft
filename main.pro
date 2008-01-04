TEMPLATE = subdirs
CONFIG += release ordered
SUBDIRS += src/exceptions \
		   src/plugininterface \
		   src/settingsdialog \
		   src \
		   src/plugins/http \
		   src/plugins/updater \
		   src/plugins/batcher \
		   src/plugins/cron \
		   src/plugins/torrent
