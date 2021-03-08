export TERMUX_PREFIX=$PREFIX
export TERMUX_PKG_EXTRA_CONFIGURE_ARGS="--with-workdir=$TERMUX_PREFIX/var/lib/cfengine --without-pam --without-selinux-policy --without-systemd-service --with-lmdb=$TERMUX_PREFIX --with-openssl=$TERMUX_PREFIX --with-yaml=$TERMUX_PREFIX --with-pcre=$TERMUX_PREFIX --with-prefix=$TERMUX_PREFIX --with-libxml2=$TERMUX_PREFIX"
export LDFLAGS+=" -landroid-glob"
NO_CONFIGURE=1 ./autogen.sh $TERMUX_PKG_EXTRA_CONFIGURE_ARGS --prefix=$TERMUX_PREFIX/var/lib/cfengine --bindir=$TERMUX_PREFIX/bin

./configure $TERMUX_PKG_EXTRA_CONFIGURE_ARGS
make -j4 install
cd ../masterfiles
./autogen.sh --prefix=$TERMUX_PREFIX/var/lib/cfengine --bindir=$TERMUX_PREFIX/bin
make install
#./configure --enable-maintainer-mode --without-pam --with-libyaml
