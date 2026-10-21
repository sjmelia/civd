# Host commands for the period DOS project in src/. The DOS build itself is
# src/MAKEFILE run by the supplied MAKE 4.07 inside DOSBox-X; these recipes
# install tools, copy sources in, check the results and build the book.

disks := "data/Microsoft C 5.1 Optimizing Compiler (5.25)"
out := "build/dos"
tools := "build/period-tools"

export SDL_AUDIODRIVER := "dummy"
export SDL_VIDEODRIVER := env("SDL_VIDEODRIVER", "dummy")

# List recipes.
default:
    @just --list

# Download the compiler disk images from WinWorld if they are not present.
download-tools:
    sh tools/download_msc51.sh "{{disks}}"

# Install the period tools; fetch missing default compiler images automatically.
setup images=disks:
    #!/bin/sh
    # Copy each needed disk into BIN, LIB or INCLUDE inside DOSBox-X. LINK 3.65
    # (02113) goes first and is renamed so LINK 5.01 (01524) becomes LINK.EXE.
    # MASM 5.10 comes from Microsoft's MS-DOS repository.
    set -eu
    if [ "{{images}}" = "{{disks}}" ]; then
        sh tools/download_msc51.sh "{{images}}"
    fi
    rm -rf "{{tools}}"
    mkdir -p "{{tools}}/BIN" "{{tools}}/LIB" "{{tools}}/INCLUDE/SYS"
    curl -fsSL -o "{{tools}}/BIN/MASM.EXE" https://raw.githubusercontent.com/microsoft/MS-DOS/2d04cacc5322951f187bb17e017c12920ac8ebe2/v4.0/src/TOOLS/MASM.EXE
    disk() { printf 'imgmount a "%s" -t floppy -ro\r\n' "$(ls "$PWD/{{images}}"/*"($1).img")"; }
    {
        disk 02113; printf 'copy A:\\*.* C:\\BIN\r\nren C:\\BIN\\LINK.EXE LINK365.EXE\r\nimgmount -u a\r\n'
        for n in 01521 01522 01523 01524; do disk $n; printf 'copy A:\\*.* C:\\BIN\r\nimgmount -u a\r\n'; done
        for n in 01527 01529; do disk $n; printf 'copy A:\\*.LIB C:\\LIB\r\nimgmount -u a\r\n'; done
        disk 01526; printf 'copy A:\\INCLUDE\\*.H C:\\INCLUDE\r\ncopy A:\\INCLUDE\\SYS\\*.H C:\\INCLUDE\\SYS\r\nimgmount -u a\r\n'
    } >"{{tools}}/SETUP.BAT"
    dosbox-x -defaultconf -defaultmapper -conf dosbox.conf -silent -fastlaunch \
        -nopromptfolder -nolog -time-limit 60 -c 'mount c "{{tools}}"' -c c: \
        -c 'call SETUP.BAT' -c exit >build/setup.log 2>&1
    rm "{{tools}}/SETUP.BAT"
    cd "{{tools}}" && sha256sum --quiet -c "{{justfile_directory()}}/period-tools.sha256"
    echo 'Period tools installed and verified.'

# Format src/*.C and src/*.H in place with GNU indent (.indent.pro).
fmt:
    #!/bin/sh
    # indent mishandles CR, so it formats LF text and CRLF is restored.
    set -eu
    for file in src/*.C src/*.H; do
        tr -d '\r' <"$file" | indent | sed 's/$/\r/' >"$file.indent"
        mv "$file.indent" "$file"
    done

# Check indent style and CRLF endings in src/ without changing files.
lint:
    #!/bin/sh
    set -eu
    status=0
    tmp=$(mktemp)
    trap 'rm -f "$tmp"' EXIT
    for file in src/*.C src/*.H; do
        tr -d '\r' <"$file" | indent | sed 's/$/\r/' >"$tmp"
        cmp -s "$tmp" "$file" || { echo "$file: run just fmt" >&2; status=1; }
    done
    for file in src/*.C src/*.H src/*.ASM src/*.INC src/MAKEFILE src/LINK.RSP; do
        if grep -q -v "$(printf '\r')\$" "$file"; then
            echo "$file: line without CRLF ending" >&2
            status=1
        fi
    done
    exit $status

# Lint, compile and build the book: everything.
build: lint compile book

# Compile src/ with MAKE in DOSBox-X under build/dos/ (incremental).
compile:
    #!/bin/sh
    set -eu
    [ -f "{{tools}}/BIN/MAKE.EXE" ] || { echo 'Run just setup first.' >&2; exit 1; }
    (cd "{{tools}}" && sha256sum --quiet -c "{{justfile_directory()}}/period-tools.sha256")
    mkdir -p "{{out}}"
    exec 9>build/build.lock
    flock -n 9 || { echo 'FAIL: another build is running.' >&2; exit 1; }
    # Drop copies of removed sources; copy only changed files, so their new
    # mtimes tell MAKE exactly what to rebuild whatever the source mtimes are.
    for copy in "{{out}}"/*.C "{{out}}"/*.H "{{out}}"/*.ASM "{{out}}"/*.INC; do
        [ -e "$copy" ] && [ ! -e "src/$(basename "$copy")" ] && rm -f "$copy"
    done
    for file in src/*.C src/*.H src/*.ASM src/*.INC src/MAKEFILE src/LINK.RSP; do
        cmp -s "$file" "{{out}}/$(basename "$file")" || cp "$file" "{{out}}/"
    done
    rm -f "{{out}}"/CIV.EXE "{{out}}"/CIVRAW.EXE "{{out}}"/*.LOG
    dosbox-x -defaultconf -defaultmapper \
        -conf dosbox.conf -silent -fastlaunch -nopromptfolder -nolog \
        -time-limit 120 -c 'mount c "{{justfile_directory()}}/{{tools}}"' \
        -c 'mount d "{{justfile_directory()}}/{{out}}"' \
        -c 'd:' -c 'set PATH=C:\BIN;Z:\' -c 'set INCLUDE=C:\INCLUDE' \
        -c 'make MAKEFILE >MAKE.LOG' -c exit >build/dosbox.log 2>&1
    # Period MAKE can continue after a failed compiler or LINK invocation.
    # Reject diagnostics even if an older object produced an executable.
    if grep -E '(fatal error|error) [A-Z][0-9]{4}:|There were [1-9][0-9]* errors detected' "{{out}}"/*.LOG; then
        echo 'FAIL: compiler or linker errors; see build logs.' >&2
        exit 1
    fi
    rebuilt=$(grep -c -E '^ +(loadfix -[0-9]+ )?(cl|masm) ' "{{out}}/MAKE.LOG" || true)
    if [ ! -f "{{out}}/CIV.EXE" ]; then
        echo "FAIL: build failed; see {{out}}/MAKE.LOG, MSC.LOG, MASM.LOG, LINK.LOG." >&2
        exit 1
    fi
    echo "Built {{out}}/CIV.EXE ($rebuilt objects rebuilt)."

# Delete the DOS build and the book (installed tools are kept).
clean:
    rm -rf "{{out}}" build/book

# Build the mdBook into build/book.
book:
    mdbook build

# Serve the mdBook with live reload.
serve-book:
    mdbook serve --open

# Build from scratch and require the original raw LINK output and CIV.EXE.
verify: clean compile
    #!/bin/sh
    # LINK output is kept unmodified as CIVRAW.EXE; CRTEDIT's four-byte
    # startup edit is the only change between it and CIV.EXE.
    cd "{{out}}"
    if sha256sum --quiet -c "{{justfile_directory()}}/civ.sha256"; then
        echo 'PASS: CIV.EXE (474.05), MGRAPHIC.EXE, ASOUND.CVL and MISC.EXE match the originals.'
    else
        echo 'FAIL: output differs from the original.' >&2
        exit 1
    fi

# Narrative Docco pages; install host Node.js/npm and run npm ci once first.
story:
    npm run story
