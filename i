#!/usr/bin/env sh
set -ex
make -j8
sudo make install # so that lldb gets debug symbols in libraries, I guess it pulls from installed base instead of local, libtool help?
./libtool --mode=execute lldb cf-agent/cf-agent
exit 0
cf-agent/cf-agent -KIf ./t.cf
exit 0
cf-agent/cf-agent -Kdf ./t.cf | tee log
#cf-agent/cf-agent -KIf ./t.cf --show-evaluated-vars | grep platform_default
exit 0
./autogen.sh --with-systemd-service=no
#./autogen.sh  --with-systemd-service=no --with-systemd-socket=no
make -j8 CFLAGS="-Werror -Wall"
make -C tests/unit check
# use clang instead of gcc so I can debug with lldb?
  514  which clang
  515  which clang++
  516  export CC=/usr/bin/clang
  517  export CXX=/usr/bin/clang++
  518  ./configure --enable-debug
./libtool --mode=execute lldb ./cf-agent/cf-agent
