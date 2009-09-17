#!/usr/bin/env bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir resolver_TEST_blockers_dir || exit 1
cd resolver_TEST_blockers_dir || exit 1

mkdir -p build
mkdir -p distdir
mkdir -p installed

mkdir -p repo/{profiles/profile,metadata}

cd repo
echo "repo" > profiles/repo_name
: > metadata/categories.conf

# hard
echo 'hard' >> metadata/categories.conf

mkdir -p 'packages/hard/target'
cat <<END > packages/hard/target/target-1.exheres-0
SUMMARY="target"
PLATFORMS="test"
SLOT="0"
DEPENDENCIES="
    ( !hard/a-pkg[<2] !hard/z-pkg[<2] )
    "
END

mkdir -p 'packages/hard/a-pkg'
cat <<END > packages/hard/a-pkg/a-pkg-2.exheres-0
SUMMARY="dep"
PLATFORMS="test"
SLOT="0"
END

mkdir -p 'packages/hard/z-pkg'
cat <<END > packages/hard/z-pkg/z-pkg-2.exheres-0
SUMMARY="dep"
PLATFORMS="test"
SLOT="0"
END

cd ..
