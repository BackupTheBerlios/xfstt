#!/bin/sh
#
# Start or stop xfstt
# by Stephen Carpenter <sjc@debian.org>
# and Gergely Egervary <mauzi@lin.lkg.c3.hu>
#
# Sun Sep 27 19:42:03 CEST 1998

PATH=/bin:/usr/bin:/sbin:/usr/sbin
XFSTT=/usr/X11R6/bin/xfstt
PIDFILE=/var/run/xfstt.pid

# Change this to change the default port
portno=7101
newuser=nobody
daemon="--daemon"

ARGS="--port $portno $daemon --user $newuser"


test -x "$XFSTT" || exit 0


case "$1" in
    start)
    
	    echo -n "Starting X TrueType Font Server: xfstt"
	    start-stop-daemon --start --quiet --exec $XFSTT -- $ARGS 
	    echo "."	

    ;;

    stop)
	    echo -n "Stopping X TrueType Font Server: xfstt"
	    start-stop-daemon --stop --quiet --pidfile $PIDFILE --exec $XFSTT ||  echo -n " not running" 
	    echo "."	

    ;;

    force-reload|restart)
	    echo -n "Reloading X True Type Font Server configuration..."
	    $0 stop
	    $XFSTT --sync
	    $0 start
    ;;

    *)
	echo "Usage: /etc/init.d/xfstt {start|stop|force-reload|restart}" >&2
	exit 1
    ;;
esac

exit 0
