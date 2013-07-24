#!/bin/sh

# Set up the environment
A="/etc/xdg/openbox/environment"
test -r $A && . $A
A="${XDG_CONFIG_HOME:-"$HOME/.config"}/openbox/environment"
test -r $A && . $A

# Run Openbox, and have it run the autostart stuff
exec /usr/bin/openbox --startup "/usr/libexec/openbox-autostart OPENBOX" "$@"

