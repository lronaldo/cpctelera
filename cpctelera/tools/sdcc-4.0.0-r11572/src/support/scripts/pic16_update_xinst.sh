#!/bin/sh

GPUTILS="$1";
PIC16DEVICES="${2:-pic16devices.txt}";

usage()
{
  echo "Usage: $0 path-to-gputils-sources name-of-pic16devices.txt";
  echo "";
  exit 1;
}

if [ ! -f "$GPUTILS/libgputils/gpprocessor.c" ]; then
  echo "$GPUTILS/libgputils/gpprocessor.c not found.";
  usage;
fi;

if [ ! -f "$PIC16DEVICES" ]; then
  echo "$PIC16DEVICES not found.";
  usage;
fi;

BASE=$(readlink -f $(dirname "$0"));
SEDSCRIPT="update_xinst.sed";
FULL="${PIC16DEVICES}-full";
OUTPUT="${PIC16DEVICES}-xinst";

grep 'PROC_CLASS_PIC16' "$1/libgputils/gpprocessor.c" \
         | sed -e '/"p1\w*"/ { s/^.*"p\(1\w*\)".*,\s*\([01]\)\s*}[^}]*$/\1 \2/; p }; d' \
         | while read p xinst; do \
                printf '/name\s*'"$p"'\s*$/ {a\\\nXINST       '"$xinst"'\n}\n'; \
           done > "$SEDSCRIPT";
perl "$BASE/optimize_pic16devices.pl" -u "$PIC16DEVICES" | \
         grep -v '^XINST\s*[01]\s*$' | \
         sed -f "$SEDSCRIPT" > "$FULL";
rm -f "$SEDSCRIPT";
perl "$BASE/optimize_pic16devices.pl" -o "$FULL" > "$OUTPUT";
rm -f "$FULL";
if diff -up "$PIC16DEVICES" "$OUTPUT"; then
  echo "No update required.";
  rm -f "$OUTPUT";
else
  echo "Update $PIC16DEVICES from $OUTPUT [y/N]? ";
  read answer;
  case ${answer:-n} in
    y|Y|y*|Y*)
      echo "Updating ...";
      mv "$OUTPUT" "$PIC16DEVICES";
      ;;
    *)
      echo "Not updating.";
      ;;
  esac;
fi;
