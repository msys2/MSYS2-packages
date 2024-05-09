#!/bin/sh

set -e

die () {
	echo "$*" >&2
	exit 1
}

cd "$(dirname "$0")" ||
die "Could not cd to msys2-pacman/"

git rev-parse --verify HEAD >/dev/null &&
git update-index -q --ignore-submodules --refresh &&
git diff-files --quiet --ignore-submodules &&
git diff-index --cached --quiet --ignore-submodules HEAD -- ||
die "Clean worktree required"

git rm 0*.patch ||
die "Could not remove previous patches"

base_tag=refs/tags/v"$(sed -ne 's/^pkgver=//p' <PKGBUILD)"
msys2_branch=refs/heads/msys2-${base_tag#refs/tags/}
url=https://github.com/msys2/msys2-pacman

test -d msys2-pacman ||
git clone --bare $url msys2-pacman ||
die "Could not clone msys2-pacman"

git -C msys2-pacman fetch --no-tags $url $base_tag:$base_tag $msys2_branch:$msys2_branch

git -c core.abbrev=7 \
	-c diff.renames=true \
	-c format.from=false \
	-c format.numbered=auto \
	-c format.useAutoBase=false \
	-C msys2-pacman \
	format-patch \
		--no-signature \
		--topo-order \
		--diff-algorithm=default \
		--no-attach \
		--no-add-header \
		--no-cover-letter \
		--no-thread \
		--suffix=.patch \
		--subject-prefix=PATCH \
		--output-directory .. \
		$base_tag..$msys2_branch \
		-- ':(exclude).github/' ||
die "Could not generate new patch set"

patches="$(ls -1 0*.patch)" &&
for p in $patches
do
	sed -i 's/^\(Subject: \[PATCH [0-9]*\/\)[1-9][0-9]*/\1N/' $p ||
	die "Could not fix Subject: line in $p"
done &&
git add $patches ||
die "Could not stage new patch set"

in_sources="$(echo "$patches" | sed "{s/^/        /;:1;N;s/\\n/\\\\n        /;b1}")"
in_prepare="$(echo "$patches" | tr '\n' '\\' | sed -e 's/\\$//' -e 's/\\/ &&&n  /g')"
sed -i -e "/^        0.*\.patch$/{:1;N;/[^)]$/b1;s|.*|$in_sources)|}" \
	-e "/^ *apply_git_with_msg /{:2;N;/[^}]$/b2;s|.*| apply_git_with_msg $in_prepare\\n\\}|}" \
	PKGBUILD ||
die "Could not update the patch set in PKGBUILD"

updpkgsums ||
die "Could not update the patch set checksums in PKGBUILD"

git add PKGBUILD ||
die "Could not stage updates in PKGBUILD"
