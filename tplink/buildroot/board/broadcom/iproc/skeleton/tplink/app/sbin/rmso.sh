#!/bin/sh
for SONAME in `ls /exe/lib | grep .*\.so$`
do
#  echo $SONAME
  if [ -f /exe/lib/$SONAME".0.0.0" ]; then
      rm /exe/lib/$SONAME
      ln /exe/lib/$SONAME".0.0.0" /exe/lib/$SONAME
      if [ -f /exe/lib/$SONAME".0" ]; then
        rm /exe/lib/$SONAME".0"
        ln /exe/lib/$SONAME".0.0.0" /exe/lib/$SONAME".0"
      fi
  else 
      if [ -f /exe/lib/$SONAME".0.0" ]; then
          rm /exe/lib/$SONAME
          ln /exe/lib/$SONAME".0.0" /exe/lib/$SONAME
          if [ -f /exe/lib/$SONAME".0" ]; then
            rm /exe/lib/$SONAME".0"
            ln /exe/lib/$SONAME".0.0" /exe/lib/$SONAME".0"
          fi
      else
        echo $SONAME".0.0.0 and "$SONAME".0.0 is not exist"
      fi
  fi
done
echo "done"
 