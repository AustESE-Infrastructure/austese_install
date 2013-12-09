#!/bin/sh
. ./functions/global.sh
ensure drush dis mongodb_block mongodb_block_ui mongodb_cache mongodb_migrate mongodb_queue mongodb_session mongodb_field_storage mongodb_watchdog -y
