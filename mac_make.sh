
export APP_NAME=SCVDebug
export BUNDLE_ID=ru.SCVDebug
export BUNDLE_VERSION=1.0.0
export BUNDLE_SHORT_VERSION=1.0
export BUNDLE_EXECUTABLE=${APP_NAME}

SRC_FILES=$(ls | grep -e '\.h$' -e '\.c$')
PROJECT=${APP_NAME}
BUNDLE=${APP_NAME}.app


FRAMEWORKS="-framework OpenGL -framework CoreVideo -framework Cocoa"
OBJCFLAGS="-ObjC -Wall -Wextra -Wpedantic -Werror -Wno-deprecated -std=c99 -fno-stack-protector"
#LDFLAGS="-nostdlib -nodefaultlibs"
LDFLAGS=""
SRC=macos_ws.c
FSANITITZE="-fsanitize=address"

case $1 in
  'testrender')
    rm -fr $BUNDLE
    mkdir $BUNDLE
    clang -o app -g -O0 $OBJCFLAGS  $FRAMEWORKS $LDFLAGS macos_platform.m
    ;;
  'build')
    rm -fr $BUNDLE
    mkdir $BUNDLE
    clang -o app -g -O0 $OBJCFLAGS  $FRAMEWORKS $LDFLAGS  $SRC
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
