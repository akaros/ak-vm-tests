
all: launcher ext_state_leak_test photon

launcher: launcher/launcher.c
	gcc -o bin/launcher launcher/launcher.c -static

ext_state_leak_test: ext_state_leak_test.c ext_state_leak_test.c
	gcc -o usr-bin-tests/ext_state_leak_test ext_state_leak_test.c -static
	gcc -o usr-bin-tests/ext_state_leak_test ext_state_leak_test.c -static

photon: photon/photon.c photon/photon.h
	gcc -D MAIN=1 -o usr-bin-tests/photon photon/photon.c -lm -static