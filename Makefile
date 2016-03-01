
all: launcher ext_state_leak_test photon

launcher: launcher/launcher.c
	gcc -o bin/launcher launcher/launcher.c -static

ext_state_leak_test: ext_state_leak_test/ext_state_leak_test_a.c ext_state_leak_test/ext_state_leak_test_b.c
	gcc -o usr-bin-tests/ext_state_leak_test_a ext_state_leak_test/ext_state_leak_test_a.c -static
	gcc -o usr-bin-tests/ext_state_leak_test_b ext_state_leak_test/ext_state_leak_test_b.c -static

photon: photon/photon.c photon/photon.h
	gcc -D MAIN=1 -o usr-bin-tests/photon photon/photon.c -lm -static