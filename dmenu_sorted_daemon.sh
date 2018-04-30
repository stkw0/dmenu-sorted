#!/bin/bash 

FNAME=~/.cache/dmenu_sorted
WAIT_FILES=""
for i in $(echo "$PATH" | tr ':' '\n'); do
   stat "$i" &> /dev/null && WAIT_FILES+=" $i"
done

DMENU_EXISTS=true
stat $FNAME &> /dev/null || DMENU_EXISTS=false

if ! $DMENU_EXISTS; then
   TMP_FILE=$(mktemp)
   find $WAIT_FILES -type f -executable > "$TMP_FILE"
   for i in $(sort -u "$TMP_FILE"); do
      NAME=$(basename $i)
      echo "0 $NAME" >> $FNAME
   done
   rm "$TMP_FILE"
fi

while true; do
   EVENT=$(inotifywait -r -q --format "%f %e" -e "delete,create,move" $WAIT_FILES)
   EVENT_TYPE=$(echo $EVENT | cut -d' ' -f2)
   EVENT_FILE=$(basename "$(echo $EVENT | cut -d' ' -f1 | cut -d# -f1)")
   if [ "$EVENT_TYPE" == "CREATE" ]; then
      grep " $EVENT_FILE$" "$FNAME" || echo "0 $EVENT_FILE" >> $FNAME
   elif [ "$EVENT_TYPE" == "DELETE" ]; then
      grep -v " $EVENT_FILE$" "$FNAME" > /tmp/new_dmenu_sorted
      mv /tmp/new_dmenu_sorted ~/.cache/dmenu_sorted
   fi
done

