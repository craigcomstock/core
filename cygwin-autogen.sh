#CC=x86_64-w64-mingw32-gcc \
#CPPFLAGS=" -mwin32 " \
#CFLAGS=" -mwin32 " \
CPPFLAGS=" -mwin32 -I/mingw64/include " \
CFLAGS=" -mwin32 -I/mingw64/include " \
LDFLAGS=" -L/mingw64/lib " \
./autogen.sh \
--without-pam
#CPPFLAGS=" -pthread " \
#CFLAGS=" -pthread " \
#LDFLAGS=" -lpthread " \
#CC=x86_64-w64-mingw32-gcc ./autogen.sh --without-pam
#CPPFLAGS="-I/usr/include" CFLAGS="$CPPFLAGS" LDFLAGS="-L/usr/lib" CC=x86_64-w64-mingw32-gcc ./autogen.sh --without-pam
