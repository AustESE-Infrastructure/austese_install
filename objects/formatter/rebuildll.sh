if [ $USER = "root" ]; then
  HASREADLINK=`which readlink`
  JAVAC=`which javac`
  getjdkinclude()
  {
    if [ -n "$HASREADLINK" ]; then 
      while [ -h $JAVAC ]
      do
        JAVAC=`readlink $JAVAC`
      done
      echo `dirname $(dirname $JAVAC)`/$JDKINCLUDEDIRNAME
    else
      echo "need readlink. please install."
    fi
    return 
  }
  if [ `uname` = "Darwin" ]; then
    LIBSUFFIX="dylib"
    JDKINCLUDEDIRNAME="Headers"
  else
    LIBSUFFIX="so"
    JDKINCLUDEDIRNAME="include"
  fi
  JDKINC=`getjdkinclude`
  gcc -c -DHAVE_EXPAT_CONFIG_H -DHAVE_MEMMOVE -DJNI -I$JDKINC -Iinclude -Iinclude/STIL -Iinclude/AESE -O0 -Wall -g3 -fPIC src/*.c src/AESE/*.c src/STIL/*.c 
  gcc *.o -shared -o libAeseFormatter.$LIBSUFFIX
  mv libAeseFormatter.$LIBSUFFIX /usr/local/lib/
  rm *.o
else
	echo "Need to be root. Did you use sudo?"
fi

