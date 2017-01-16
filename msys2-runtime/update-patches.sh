#!/bin/sh

die () {
	echo "$*" >&2
	exit 1
}

cd "$(dirname "$0")" ||
die "Could not cd to msys2-runtime/"

base_tag=refs/tags/cygwin-"$(sed -ne 'y/./_/' -e 's/^pkgver=//p' <PKGBUILD)"-release

git rev-parse --verify HEAD >/dev/null &&
git update-index -q --ignore-submodules --refresh &&
git diff-files --quiet --ignore-submodules &&
git diff-index --cached --quiet --ignore-submodules HEAD -- ||
die "Clean worktree required"

git rm 0*.patch ||
die "Could not remove previous patches"

git -c core.abbrev=7 -C src/msys2-runtime format-patch -o ../.. --signature=2.9.0 \
	$base_tag.. ||
die "Could not generate new patch set"

patches="$(ls 0*.patch)" &&
git add $patches ||
die "Could not stage new patch set"

in_sources="$(echo "$patches" | sed "{s/^/        /;:1;N;s/\\n/\\\\n        /;b1}")"
in_prepare="$(echo "$patches" |
	sed "{s|^|  git am --committer-date-is-author-date \"\${srcdir}\"/|;:1;N;
	 s|\\n|\\\\n  git am --committer-date-is-author-date \"\${srcdir}\"/|;b1}")"
sed -i -e "/^        0.*\.patch$/{:1;N;/[^)]$/b1;s|.*|$in_sources)|}" \
	-e "/^  git am.*\.patch$/{:2;N;/[^}]$/b2;s|.*|$in_prepare\\n\\}|}" \
	PKGBUILD ||
die "Could not update the patch set in PKGBUILD"

updpkgsums ||
die "Could not update the patch set checksums in PKGBUILD"

git add PKGBUILD ||
die "Could not stage updates in PKGBUILD"
