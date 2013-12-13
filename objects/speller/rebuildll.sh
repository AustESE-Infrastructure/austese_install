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
      exit    
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
  gcc -c -DHAVE_MEMMOVE -DJNI -I$JDKINC -Iinclude -O0 -Wall -g3 -fPIC src/aesespeller.c
  gcc *.o -shared -laspell -o libAeseSpeller.$LIBSUFFIX
  cp libAeseSpeller.$LIBSUFFIX /usr/local/lib
else
	echo "Need to be root. Did you use sudo?"
fi

