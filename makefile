.PHONY : echo-client echo-server clean install uninstall android-install android-uninstall

all: echo-client echo-server

echo-client:
	cd echo-client; make; cd ..

echo-server:
	cd echo-server; make; cd ..

clean:
	cd echo-client; make clean; cd ..
	cd echo-server; make clean; cd ..

install:
	sudo cp bin/echo-client /usr/local/sbin
	sudo cp bin/echo-server /usr/local/sbin

uninstall:
	sudo rm /usr/local/sbin/echo-client /usr/local/sbin/echo-server

android-install:
	adb push bin/echo-client bin/echo-server /data/local/tmp
	adb exec-out "su -c 'mount -o rw,remount /system'"
	adb exec-out "su -c 'cp /data/local/tmp/echo-client /data/local/tmp/echo-server /system/xbin'"
	adb exec-out "su -c 'chmod 755 /system/xbin/echo-client'"
	adb exec-out "su -c 'chmod 755 /system/xbin/echo-server'"
	adb exec-out "su -c 'mount -o ro,remount /system'"
	adb exec-out "su -c 'rm /data/local/tmp/echo-client /data/local/tmp/echo-server'"

android-uninstall:
	adb exec-out "su -c 'mount -o rw,remount /system'"
	adb exec-out "su -c 'rm /system/xbin/echo-client /system/xbin/echo-server'"
	adb exec-out "su -c 'mount -o ro,remount /system'"
