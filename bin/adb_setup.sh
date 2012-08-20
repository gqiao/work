for i in `find *.apk`; do adb install -r ${i}; mv ${i} /work/apk/; done
