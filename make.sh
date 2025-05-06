CFLAGS="-Wall -Wextra -Wpedantic -Werror -Wno-deprecated -std=c99"
LDFLAGS="-ffreestanding -nostartfiles -nostdlib -fno-builtin -fno-stack-protector -fno-unwind-tables -fno-asynchronous-unwind-tables"
SRC="linux_app.c start.s"
SRCWS="linux_ws.c start.s"
OUT=app

case $1 in
  'build')
    rm $OUT
    clang -o $OUT -O0 -g $CFLAGS $LDFLAGS $SRC
    ;;
  'buildws')
    rm $OUT
    gcc -o $OUT -O0 -g $CFLAGS $LDFLAGS $SRCWS
    ;;
  'test')
    rm test
    tcc -s -O0 -g $CFLAGS $LDFLAGS test.c -o test
    ;;
  'fmt')
      for file in $SRC_FILES; do
        echo $file
				clang-format --style=Mozilla $file >reformatted
			  cp reformatted $file
			done
    ;;
  'run')
    ./${BUNDLE}/${PROJECT}
    ;;
esac
