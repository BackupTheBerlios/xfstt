#!/bin/bash

###
# FILE    : SlackMe.sh
# INFOS   : Build the XFSTT Slackware Package
# AUTHOR  : LiNuCe (Lucien NARDINI) <massilia98@yahoo.com>
# VERSION : hmmm ... let's try 1.00  and hope it works ;-)
###

if [ "$UID" != "0" ]
then
   echo ""
   echo "###"
   echo "# Execute this script as root to build the XFSTT package."
   echo "# READ THE DOC IF YOU WANT XFSTT TO RUN PROPERLY ON YOUR SYSTEM !" 
   echo "###"
   echo ""
   exit 1
fi

cd ..
make xfstt
cd slackware 

mkdir    slackroot/ 
mkdir -p slackroot/etc/rc.d/
mkdir -p slackroot/usr/doc/xfstt-1.1/
mkdir -p slackroot/usr/X11R6/bin/ 
mkdir -p slackroot/usr/X11R6/lib/fonts/ttf/ 
mkdir -p slackroot/usr/X11R6/man/man1/
mkdir -p slackroot/var/cache/xfstt/ 
mkdir -p slackroot/install/

cp ../FAQ       slackroot/usr/doc/xfstt-1.1/
cp ../CHANGES   slackroot/usr/doc/xfstt-1.1/
cp ../COPYING   slackroot/usr/doc/xfstt-1.1/
cp ../README*   slackroot/usr/doc/xfstt-1.1/
cp ../THANKS*   slackroot/usr/doc/xfstt-1.1/
cp ../xfstt.lsm slackroot/usr/doc/xfstt-1.1/
cp ../xfstt     slackroot/usr/X11R6/bin/
cp ../xfstt.1x  slackroot/usr/X11R6/man/man1/
cp ./rc.xfstt   slackroot/etc/rc.d/
cp ./doinst.sh  slackroot/install/doinst.sh

chmod 700 slackroot/etc/rc.d/rc.xfstt
chmod 700 slackroot/install/doinst.sh

cd slackroot
echo y | makepkg xfstt.tgz
mv xfstt.tgz ..
cd ..
rm -rf slackroot

echo ""
echo "###"
echo "# Now, try \"installpkg xfstt.tgz\" to install XFSTT"
echo "# on your system. As root obviously ..."
echo "###"
echo ""
exit 0
