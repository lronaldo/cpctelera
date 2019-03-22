#!/bin/sh
echo java -jar "`dirname $0`/setAxmlPkgName.jar" "$@"
java -jar "`dirname $0`/setAxmlPkgName.jar" "$@"
