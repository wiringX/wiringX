#!/usr/bin/env bash

rm -r debs/*/* 2>/dev/null

cmake .
make

mkdir -p debs/armhf/usr/local/include/
mkdir -p debs/armhf/usr/local/lib/
cat <<\EOF > debs/armhf/control
Package: libwiringx
Version: 1.0
Section: utils
Priority: optional
Architecture: armhf
Installed-Size: @size@
Maintainer: CurlyMo <info@pilight.org>
Description: test

EOF

cp libwiringX.{a,so} debs/armhf/usr/local/lib/
cp wiringX.h debs/armhf/usr/local/include/

SIZE=$(du -c debs/armhf/usr/ | grep total | awk '{print $1}');

sed -e "s/Package: \(.*\)/Package: libwiringx/g" debs/armhf/control > debs/armhf/control.new
mv debs/armhf/control.new debs/armhf/control

sed -e "s/@size@/$SIZE/g" debs/armhf/control > debs/armhf/control.new
mv debs/armhf/control.new debs/armhf/control

echo 2.0 > debs/armhf/debian-binary

DIR="debs/armhf/";
VERSION=$(git log --oneline | wc -l);
VERSION=$(echo $VERSION-$(git log --oneline | head -n 1 | awk '{print $1}'));
DESC="GPIO interface for the Hummingboard and Raspberry Pi";
DNAME="libwiringx";
cd $DIR;
if [ $? -eq 0 ]; then
	ARCH=$(sed -ne "s/Architecture: \(.*\)/\1/p" control)
	sed -e "s/Package: \(.*\)/Package: $DNAME/g" control > control.new
	mv control.new control
	sed -e "s/Version: \(.*\)/Version: $VERSION/g" control > control.new
	mv control.new control
	sed -e "s/Section: \(.*\)/Section: utils/g" control > control.new
	mv control.new control
	sed -e "s/Description: \(.*\)/Description: $DESC/g" control > control.new
	mv control.new control
	chmod 755 control
cat <<\EOF > postinst
#!/bin/bash

if [[ $1 == "configure" ]]; then
	true
fi
EOF

cat <<\EOF > postrm
#!/bin/bash

if [[ $1 == "purge" ]]; then

	if [ -f /usr/local/lib/wiringx* ]; then
		rm /usr/local/lib/wiringx* 1>/dev/null 2>/dev/null;
	fi
	
	if [ -f /usr/local/include/wiringx.h ]; then
		rm /usr/local/include/wiringx.h 1>/dev/null 2>/dev/null;
	fi	

fi
EOF
		
cat <<\EOF > preinst
#!/bin/bash

if [ $1 == "install" -o $1 == "upgrade" ]; then
	true
fi
EOF

cat <<\EOF > prerm
#!/bin/bash

if [[ $1 == "remove" || $1 == "purge" ]]; then
	true;
fi
EOF

			find ./ -type f -print0 | xargs --null strip 2>/dev/null

			find usr -type f | xargs -L 1 md5sum > md5sums
			sed -e "s/ /  /g" md5sums > md5sums.new
			mv md5sums.new md5sums

			rm control.tar.gz 2>/dev/null
			tar -czf control.tar.gz control md5sums postinst postrm preinst prerm

			chown -R 0:0 usr
			tar -czf data.tar.gz usr

			rm *.deb 2>/dev/null;
			ar rcs tmp.deb debian-binary control.tar.gz data.tar.gz
			mv tmp.deb "libwiringx-$VERSION.deb";
			mv *.deb ../../
			cd ../../
		fi