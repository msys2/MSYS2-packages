#!/bin/bash

args=''
tmp=$(mktemp -d --tmpdir namcap.XXXXXXXXXX)
cleanup() {
	rm -rf "${tmp}"
}
trap 'cleanup' 0

for arg in "${@}"; do
	if echo "${arg}" | grep -q -E "^.+\.pkg\.tar\..+$" && [ -f "${arg}" ]; then

		extra_opts=''
		case "${arg##*.}" in
			gz|z|Z) cmd='gzip' ;;
			bz2|bz) cmd='bzip2' ;;
			xz)     cmd='xz' ;;
			lzo)    cmd='lzop' ;;
			lrz)    cmd='lrzip'
				extra_opts="-q -o -" ;;
			lz4)    cmd='lz4'
				extra_opts="-q" ;;
			lz)     cmd='lzip'
				extra_opts="-q" ;;
			zst)    cmd='zstd'
				extra_opts="-q" ;;
			*)      echo 'Unsupported compression'; exit 1;;
		esac

		tar="${tmp}/$(basename "${arg%.*}")"
		$cmd -dcf $extra_opts "${arg}" > "${tar}"

		args="${args} ${tar}"
	else
		args="${args} ${arg}"
	fi
done

/usr/bin/env python3 -m namcap ${args}
