Summary: X Font Server for *.ttf fonts
Name: xfstt
Version: 0.9.6
Release: 2
Copyright: LGPL
Group: X11/Utilities
Source: ftp://sunsite.unc.edu/pub/Linux/X11/fonts/xfstt-0.9.6.tgz
Source1: xfstt.init
BuildRoot: /tmp/xfstt-root

%description
xfstt means "X11 Font Server for TrueType fonts".
TT fonts are generally regarded to be the best scalable fonts
for displays. Applications that need scalable fonts for display
on low resolution devices like screens benefit most.

%pre
# adds "inet/127.0.0.1:7100" to the list of searchable fonts
if [ -r /etc/X11/XF86Config ]; then
   if grep "FontPath.*7100" /etc/X11/XF86Config > /dev/null
   then
      /bin/true
   else
      LINE=`grep -n "FontPath" /etc/X11/XF86Config * cut -d: -f1`
      TOTAL=`wc -l < /etc/X11/XF86Config`
      FONTLIST=`grep "FontPath" /etc/X11/XF86Config * cut -d'"' -f2`
      head -`expr $LINE - 1` /etc/X11/XF86Config > /tmp/XF86Config.$$ \
      && echo "   FontPath   \"$FONTLIST,inet/127.0.0.1:7100\"" >>
/tmp/XF86Config.$$ \
      && tail -`expr $TOTAL - $LINE` /etc/X11/XF86Config >> /tmp/XF86Config.$$
\
      && cat /tmp/XF86Config.$$ > /etc/X11/XF86Config \
      && rm /tmp/XF86Config.$$
   fi
fi

%prep
%setup -n xfstt

%build
make xfstt

%install
mkdir -p
$RPM_BUILD_ROOT/{etc/rc.d/{init.d,rc{0,1,2,3,5,6}.d},usr/{ttfonts,X11R6/bin}}
install -m755 -o root -g root xfstt $RPM_BUILD_ROOT/usr/X11R6/bin/xfstt
install -m755 -o root -g root $RPM_SOURCE_DIR/xfstt.init
$RPM_BUILD_ROOT/etc/rc.d/init.d/xfstt
ln -sf ../init.d/xfstt $RPM_BUILD_ROOT/etc/rc.d/rc0.d/K31xfstt
ln -sf ../init.d/xfstt $RPM_BUILD_ROOT/etc/rc.d/rc1.d/K31xfstt
ln -sf ../init.d/xfstt $RPM_BUILD_ROOT/etc/rc.d/rc2.d/S95xfstt
ln -sf ../init.d/xfstt $RPM_BUILD_ROOT/etc/rc.d/rc3.d/S95xfstt
ln -sf ../init.d/xfstt $RPM_BUILD_ROOT/etc/rc.d/rc5.d/S95xfstt
ln -sf ../init.d/xfstt $RPM_BUILD_ROOT/etc/rc.d/rc6.d/K31xfstt

%clean
rm -rf $RPM_BUILD_ROOT

%files
%doc *.txt
%config /etc/rc.d/init.d/xfstt
%config /etc/rc.d/rc0.d/K31xfstt
%config /etc/rc.d/rc1.d/K31xfstt
%config /etc/rc.d/rc2.d/S95xfstt
%config /etc/rc.d/rc3.d/S95xfstt
%config /etc/rc.d/rc5.d/S95xfstt
%config /etc/rc.d/rc6.d/K31xfstt
/usr/X11R6/bin/xfstt
%dir /usr/ttfonts
