#!/usr/bin/env bash
# Source this file only in the prepared cloud instance. Normal Linux uses README apt packages.
export PKG_CONFIG_PATH=/workspace/qt6-local/root/usr/lib/x86_64-linux-gnu/pkgconfig${PKG_CONFIG_PATH:+:$PKG_CONFIG_PATH}
export LD_LIBRARY_PATH=/workspace/qt6-local/root/usr/lib/x86_64-linux-gnu${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}
export QT_PLUGIN_PATH=/workspace/qt6-local/root/usr/lib/x86_64-linux-gnu/qt6/plugins
