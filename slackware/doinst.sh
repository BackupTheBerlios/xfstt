#!/bin/ash

###
# FILE    : doinst.sh
# INFOS   : Slackware install script for XFSTT
# AUTHOR  : LiNuCe (Lucien NARDINI) <massilia98@yahoo.com>
# LICENSE : GNU GPL
###

### 
# E-mail me if you think permissions are wrong ! 
###

useradd xfstt -d /var/cache/xfstt

mkdir -p          /usr/X11R6/lib/fonts/ttf/
chmod 740         /usr/X11R6/lib/fonts/ttf/
chown xfstt.root /usr/X11R6/lib/fonts/ttf/

mkdir -p         /var/cache/xfstt/
chmod 740        /var/cache/xfstt/
chown xfstt.root /var/cache/xfstt/
