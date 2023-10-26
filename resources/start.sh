#!/usr/bin/bash

#export GST_DEBUG="*:2"

SCRIPT=$(realpath -s "$0")
SCRIPTPATH=$(dirname "$SCRIPT")

$SCRIPTPATH/gsttogglesources \
  --uri1=rtsp://admin:Admin12345@reg.fuzzun.ru:50232/ISAPI/Streaming/Channels/101 \
  --uri2=file://$SCRIPTPATH/matrix.avi \
  --step=5000
