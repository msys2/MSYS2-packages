# Excerpted from src/platform/cygwin.sh
# Copyright (C) 2014 Jason A. Donenfeld <Jason@zx2c4.com>. All Rights Reserved.
# This file is licensed under the GPLv2+. Please see COPYING for more information.

clip() {
	local sleep_argv0="password store sleep on display $DISPLAY"
	pkill -f "^$sleep_argv0" 2>/dev/null && sleep 0.5
	local before="$($BASE64 < /dev/clipboard)"
	echo -n "$1" > /dev/clipboard
	(
		( exec -a "$sleep_argv0" sleep "$CLIP_TIME" )
		local now="$($BASE64 < /dev/clipboard)"
		[[ $now != $(echo -n "$1" | $BASE64) ]] && before="$now"
		echo "$before" | $BASE64 -d > /dev/clipboard
	) >/dev/null 2>&1 & disown
	echo "Copied $2 to clipboard. Will clear in $CLIP_TIME seconds."
}
