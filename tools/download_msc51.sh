#!/bin/sh
# Download the public WinWorld MSC 5.1 image set; preserve supplied local images.
# Source and published SHA-512:
# https://winworldpc.com/download/47e28099-4d66-18c3-9a11-c3a4e284a2ef
set -eu

msc_images=${1:-"data/Microsoft C 5.1 Optimizing Compiler (5.25)"}
msc_parts="02113 01521 01522 01523 01524 01527 01529 01526"
msc_digest=5837e2efbaa68285d5c2587e3dd2a6eae2693a21b4efc6a42b41ad22f81791c40c2da64eed5463fd9e0857c2381efe0ce32084ea6d6eab11735c057ccfc3ba80
msc_page=https://winworldpc.com/download/47e28099-4d66-18c3-9a11-c3a4e284a2ef

images_ready() {
    for msc_part in $msc_parts; do
        set -- "$msc_images"/*"($msc_part).img"
        [ "$#" -eq 1 ] && [ -f "$1" ] || return 1
    done
}

if images_ready; then
    echo "Compiler disk images already present in $msc_images."
    exit 0
fi

if command -v 7z >/dev/null 2>&1; then
    msc_extract=7z
elif command -v 7zz >/dev/null 2>&1; then
    msc_extract=7zz
else
    echo 'Install 7-Zip to download the compiler images, or use the devcontainer.' >&2
    exit 1
fi

mkdir -p data/downloads
msc_archive=data/downloads/msc51.7z
msc_tmp=$(mktemp -d data/downloads/msc51.XXXXXX)
trap 'rm -rf "$msc_tmp"' EXIT
trap 'exit 1' HUP INT TERM

archive_valid() {
    [ -f "$1" ] && printf '%s  %s\n' "$msc_digest" "$1" | sha512sum --status --check
}

if ! archive_valid "$msc_archive"; then
    for msc_mirror in c39ac2af-c381-c2bf-1b25-11c3a4e284a2 c3ae6ee2-8099-713d-3411-c3a6e280947e; do
        echo 'Downloading Microsoft C 5.1 from WinWorld...'
        if curl --fail --location --retry 2 --connect-timeout 20 --max-time 120 \
            --referer "$msc_page" --output "$msc_tmp/archive.7z" \
            "$msc_page/from/$msc_mirror" \
            && archive_valid "$msc_tmp/archive.7z"; then
            mv "$msc_tmp/archive.7z" "$msc_archive"
            break
        fi
    done
fi

if ! archive_valid "$msc_archive"; then
    echo "Compiler download failed or its checksum differs. Download manually from $msc_page." >&2
    exit 1
fi

"$msc_extract" x -y "-o$msc_tmp/extracted" "$msc_archive" >/dev/null
msc_bundle="$msc_tmp/extracted/Microsoft C 5.1 Optimizing Compiler (5.25)"
mkdir -p "$msc_images"
for msc_image in "$msc_bundle"/*.img; do
    [ -f "$msc_image" ] || { echo 'Compiler archive contains no disk images.' >&2; exit 1; }
    msc_name=$(basename "$msc_image")
    # Never overwrite a disk image supplied by the user.
    [ -e "$msc_images/$msc_name" ] || cp "$msc_image" "$msc_images/$msc_name"
done
images_ready || { echo "Missing or duplicate required images in $msc_images." >&2; exit 1; }
echo "Compiler archive verified and extracted to $msc_images."
