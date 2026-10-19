# Sid Meier's Civilization

Reverse-engineered source code for the MS-DOS version of the 1991 game "Sid Meier's Civilization".

> [!WARNING]
> This is a reconstruction, not MicroProse's original source. Names, comments
> and some type declarations are inferred. An exact executable match does not
> prove that these match the original source. 

The sources build with Microsoft C 5.1, MASM 5.10 and LINK 5.01. The
resulting `CIV.EXE` matches version 474.05 byte for byte. Verification also
checks the raw linker output, MCGA graphics driver, AdLib sound driver and
`MISC.EXE` against pinned hashes. The build applies a checked
four-byte startup correction, described in the
[build notes](book/src/architecture/toolchain_clues.md).

See also: [reciv](https://github.com/sjmelia/reciv) which rebuilds the game
for modern platforms.

## AI and credit

The project used LLMs from OpenAI, Anthropic and Google for code and
documentation, followed by editing and build verification.

It builds on earlier decompilation work, particularly
[OpenCiv1](https://codeberg.org/rhorvat/OpenCivOne).

## Documentation

- The [technical book](https://sjmelia.github.io/civd/book/) explains the game systems.
- The Docco story, [Reading Civilization from the inside](https://sjmelia.github.io/civd/story/), follows complete source files with commentary beside the code.

To build locally, run `just book` and, after `npm ci`, `just story`.
Open `build/book/index.html` or `build/story/index.html`.
The book's sources are in [book/](book/src/SUMMARY.md); the story's are in [story/](story/00-opening.c.md).

The build workflow publishes both to [GitHub Pages](https://sjmelia.github.io/civd/)
after checks pass on `main`. Pull requests build and check the documentation
without deploying it.

## Building

Microsoft MAKE 4.07 builds the source C and assembler files with Microsoft C 5.1, MASM 5.10 and LINK 5.01.

You can run the tools inside DOSBox-X. Mount the tools as C: and `src/` as D:, then run `make MAKEFILE` in D:. Objects, listings and logs are written beside the
sources (ignored by git):

```sh
dosbox-x -conf dosbox.conf -c 'mount c build/period-tools' \
  -c 'mount d src' -c 'd:' -c 'set PATH=C:\BIN;Z:\' -c 'set INCLUDE=C:\INCLUDE'
```

### Devcontainer

Install [Docker](https://docs.docker.com/get-started/get-docker/) and open this
repository in [VS Code](https://code.visualstudio.com/) with the
[Dev Containers extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers).
Run **Dev Containers: Reopen in Container**.

The [devcontainer](.devcontainer/devcontainer.json) installs the build dependencies.
On first creation it downloads the Microsoft C 5.1 disk images from
[WinWorld](https://winworldpc.com/download/47e28099-4d66-18c3-9a11-c3a4e284a2ef),
fetches MASM from Microsoft's [MS-DOS repository](https://github.com/microsoft/MS-DOS),
installs and checks the period tools, verifies the executable, and builds the book.
The [Dockerfile](.devcontainer/Dockerfile) lists and links the host dependencies.

Downloads stay in the ignored `data/` directory; installed tools go into
`build/period-tools/`. The archive checksum and
[installed-tool hashes](period-tools.sha256) are checked before use.
If you already have the compiler disk images, `just setup "path/to/images"`
uses your local copies.

The original game is not needed to build. Verification uses the pinned
SHA-256 hashes in [civ.sha256](civ.sha256).

### Continuous integration

The [GitHub build workflow](.github/workflows/build.yml) uses the same Dockerfile
on pushes and pull requests. It downloads and installs the tools, checks source
formatting, runs the complete executable verification, and builds the book and story.
Build logs are retained as workflow artifacts.

### Commands

```sh
just setup      # once: download missing images, install the period tools and check them
just build      # lint, compile and build the book
just compile    # incremental: MAKE rebuilds changed sources in build/dos/
just verify     # clean compile, then require the original raw LINK output and CIV.EXE
just clean      # delete build/dos/ and build/book/ (installed tools are kept)
just fmt        # format src/*.C and src/*.H with GNU indent (.indent.pro)
just lint       # check indent style and CRLF endings without changing files
just book       # build the mdBook into build/book/
just serve-book # serve the mdBook with live reload
just story      # build the Docco story (run npm ci once first)
```

`just verify` checks a clean build against the pinned hashes. Run `npm ci`
once before `just story`.

## Repository map

```text
src/            DOS C/MASM source, MAKEFILE and LINK.RSP
.devcontainer/  Build environment and automatic first-time setup
.github/        Build and verification workflow
tools/          Compiler download script and Docco story builder
justfile        Host commands: setup, build, compile, verify, clean, fmt, lint, book
dosbox.conf     DOSBox-X configuration for setup and compile
*.sha256        Pinned installed-tool and output hashes
book/           mdBook on the game's architecture and design
story/          Narrative commentary and current source includes for Docco
```
