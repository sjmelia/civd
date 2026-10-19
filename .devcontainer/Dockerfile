FROM debian:trixie-slim

# Host build dependencies (the DOS compiler is installed by `just setup`):
# DOSBox-X: https://dosbox-x.com/ (tested with 2025.02.01)
# just: https://github.com/casey/just
# GNU indent: https://www.gnu.org/software/indent/ (tested with 2.2.13)
# mdBook: https://rust-lang.github.io/mdBook/
# Node.js/npm (optional Docco story): https://nodejs.org/
# curl: https://curl.se/
# coreutils (sha256sum/sha512sum): https://www.gnu.org/software/coreutils/
# util-linux (flock): https://github.com/util-linux/util-linux
# 7-Zip (WinWorld archive extraction): https://www.7-zip.org/
RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        ca-certificates coreutils curl dosbox-x git indent just \
        util-linux 7zip passwd nodejs npm \
    && rm -rf /var/lib/apt/lists/*

# Use upstream mdBook: Debian's package disables the book's MathJax renderer.
# Hashes are the v0.5.4 release asset digests from rust-lang/mdBook.
RUN set -eu; \
    case "$(dpkg --print-architecture)" in \
        amd64) book_target=x86_64-unknown-linux-musl; \
            book_sha=5222beabd3e37dc5be0d18ff99b79058469354db5c220153a1b92db5ba12be89 ;; \
        arm64) book_target=aarch64-unknown-linux-musl; \
            book_sha=753e5c5c363ee8a56972344dcf91466f005a51db84a7aeffe427ae3ef83d6d44 ;; \
        *) echo 'mdBook binary supports amd64 and arm64 in this container.' >&2; exit 1 ;; \
    esac; \
    curl --fail --location --retry 2 --output /tmp/mdbook.tar.gz \
        "https://github.com/rust-lang/mdBook/releases/download/v0.5.4/mdbook-v0.5.4-$book_target.tar.gz"; \
    printf '%s  /tmp/mdbook.tar.gz\n' "$book_sha" | sha256sum --check; \
    tar -xzf /tmp/mdbook.tar.gz -C /usr/local/bin mdbook; \
    rm /tmp/mdbook.tar.gz

# Matching the checkout owner's UID/GID also permits running this image in CI.
ARG USER_UID=1000
ARG USER_GID=1000
RUN groupadd --gid ${USER_GID} vscode \
    && useradd --uid ${USER_UID} --gid ${USER_GID} --create-home \
        --shell /bin/bash vscode

# The build runs without a display or audio device.
ENV SDL_AUDIODRIVER=dummy SDL_VIDEODRIVER=dummy LANG=C.UTF-8
USER vscode
