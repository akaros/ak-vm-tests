
all: launcher ext_state_leak_test photon ak-kfs

launcher: launcher/launcher.c
	gcc -o bin/launcher launcher/launcher.c -static

ext_state_leak_test: ext_state_leak_test/ext_state_leak_test_a.c ext_state_leak_test/ext_state_leak_test_b.c
	gcc -o usr-bin-tests/ext_state_leak_test_a ext_state_leak_test/ext_state_leak_test_a.c -static
	gcc -o usr-bin-tests/ext_state_leak_test_b ext_state_leak_test/ext_state_leak_test_b.c -static
	echo "/usr/bin/tests/ext_state_leak_test_a" > usr-bin-tests/ext_state_leak_test_a.sh
	echo "/usr/bin/tests/ext_state_leak_test_b" > usr-bin-tests/ext_state_leak_test_b.sh

photon: photon/photon.c photon/photon.h
	gcc -D MAIN=1 -o usr-bin-tests/photon photon/photon.c -lm -static
	echo "/usr/bin/tests/photon" > usr-bin-tests/photon.sh

ak-kfs: akaros/kern/kfs/ext_state_leak_test.sh
	cp akaros/kern/kfs/ext_state_leak_test.sh $(AKAROS)/kern/kfs/ext_state_leak_test.sh