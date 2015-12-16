#!/bin/bash
nopollversion="nopoll-0.3.1.b217"
luarocksgit="https://github.com/keplerproject/luarocks.git"

# build and installation of connectivity daemon
# on kit it should automatic install dependencies
# add to start up and compile daemon

mkdir streamztemp
cd streamztemp

if [ ! -d /usr/local/lib/luarocks/rocks-5.2 ]; then
apt-get install -y lua5.2 lua5.2-dev cmake g++ git libssl-dev unzip
echo "#### LUAROCKS INSTALLING"
git clone $luarocksgit
cd luarocks
./configure --lua-version=5.2 --versioned-rocks-dir
make build
make install
cd ..
fi

if [ ! -d /usr/local/lib/luarocks/rocks-5.2/luasocket ]; then
	echo "#### LUASOCKET INSTALLING"
	luarocks install luasocket
fi
if [ ! -d /usr/local/lib/luarocks/rocks-5.2/luajson ]; then
	echo "#### LUAJSON INSTALLING"
	luarocks install luajson
fi
if [ ! -d /usr/local/lib/luarocks/rocks-5.2/luafilesystem ]; then
	echo "#### LUAFILESYSTEM INSTALLING"
	luarocks install luafilesystem
fi
if [ ! -d /usr/local/lib/luarocks/rocks-5.2/lpty ]; then
	echo "#### LUATTY INSTALLING"
	luarocks install lpty
fi

if [ ! -f /usr/lib/libnopoll.a ]; then
	echo "#### NOPOLL library"
	wget http://www.aspl.es/nopoll/downloads/$nopollversion.tar.gz
	tar -xzf $nopollversion.tar.gz
	cd $nopollversion
	./configure
	make; make install
	cp /usr/local/lib/libnopoll* /usr/lib
	cd ..
fi
cd ..
echo "#### Cleanup"
rm -rf streamztemp

echo "#### Compile Core"
cd Core
cmake ./
make; make install
chmod +x StreamZCoreRespawn.sh
cd ..
echo "#### Init.d scripts"
cp 4gstartercore /etc/init.d/4gstartercore
chmod +x /etc/init.d/4gstartercore

cp 4gmodeminit /etc/init.d/4gmodeminit
chmod +x /etc/init.d/4gmodeminit

chmod +x VZ20M.sh



update-rc.d 4gstartercore defaults
update-rc.d 4gmodeminit defaults

echo "#### Starting service"
/etc/init.d/4gstartercore restart || :
/etc/init.d/4gmodeminit restart || :
