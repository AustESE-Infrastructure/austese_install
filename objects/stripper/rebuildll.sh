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
  gcc -c -DHAVE_EXPAT_CONFIG_H -DHAVE_MEMMOVE -DJNI -I$JDKINC -Iinclude -I../formatter/include -I../formatter/include/STIL -O0 -Wall -g3 -fPIC ../formatter/src/STIL/cJSON.c src/*.c  
  gcc *.o -shared -lexpat -laspell -o libAeseStripper.$LIBSUFFIX
  cp libAeseStripper.$LIBSUFFIX /usr/local/lib
else
	echo "Need to be root. Did you use sudo?"
fi

