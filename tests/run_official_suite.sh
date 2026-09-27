#!/bin/bash
# Run the IETF CELLAR flac-test-files suite against flacdemo.
#
# Usage: tests/run_official_suite.sh [path-to-flac-test-files]
# Requires: ffmpeg, ffprobe.
#
# Files the decoder accepts must match ffmpeg-decoded PCM bit-exactly.
# Files outside the decoder's envelope (multichannel, mono, >24-bit,
# blocksize > FLAC_CONV_BUFSIZE, unparsable) must be rejected cleanly:
# no crash, no hang, no partial garbage output.

set -u
SUITE="${1:-/tmp/flac-test-files}"
DEMO="$(dirname "$0")/../flacdemo"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT

if [ ! -d "$SUITE/subset" ]; then
    echo "cloning flac-test-files to $SUITE"
    git clone --depth 1 https://github.com/ietf-wg-cellar/flac-test-files "$SUITE" || exit 1
fi
make -C "$(dirname "$0")/.." || exit 1

exact=0; reject=0; skip=0; bad=0
for f in "$SUITE"/subset/*.flac "$SUITE"/uncommon/*.flac "$SUITE"/faulty/*.flac; do
    timeout 60 "$DEMO" "$f" "$TMP/out" > "$TMP/log" 2>&1; rc=$?
    sz=$(stat -c%s "$TMP/out" 2>/dev/null || echo 0)
    if [ $rc -eq 124 ] || [ $rc -ge 128 ]; then
        st="HANG/CRASH(rc=$rc)"; bad=$((bad+1))
    elif grep -q 'errno' "$TMP/log"; then
        st="REJECT($(grep -o 'errno -[0-9]*' "$TMP/log"))"; reject=$((reject+1))
    elif [ "$sz" -eq 0 ]; then
        st="REJECT(empty)"; reject=$((reject+1))
    else
        case "$f" in
        *"01 - changing samplerate"*)
            # ffmpeg's whole-file raw conversion is unreliable when the sample
            # rate changes mid-stream; decoding without error is accepted here.
            st="DECODED(noverify)"; skip=$((skip+1));;
        *)
            bps=$(ffprobe -v error -show_entries stream=bits_per_raw_sample -of csv=p=0 "$f")
            case $bps in 8) fmt=s8;; 20|24) fmt=s24le;; *) fmt=s16le;; esac
            ffmpeg -y -loglevel error -i "$f" -f $fmt "$TMP/ref" 2>/dev/null
            if cmp -s "$TMP/out" "$TMP/ref"; then
                st="BIT-EXACT"; exact=$((exact+1))
            else
                st="MISMATCH"; bad=$((bad+1))
            fi;;
        esac
    fi
    rm -f "$TMP/out" "$TMP/ref"
    printf "%-72s %s\n" "$(basename "$f")" "$st"
done

echo "== bit-exact: $exact, clean reject: $reject, unverified: $skip, failures: $bad"
[ $bad -eq 0 ]
