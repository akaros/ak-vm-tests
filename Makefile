
all: launcher ext_state_leak_test prxcr0 xstate_detect xsaveopt_test ak-kfs

launcher: launcher/launcher.c
	gcc -o bin/launcher launcher/launcher.c -static

ext_state_leak_test: ext_state_leak_test/ext_state_leak_test_a.c ext_state_leak_test/ext_state_leak_test_b.c ext_state_leak_test/ext_state_leak_test_c.c
	gcc -o usr-bin-tests/ext_state_leak_test_a ext_state_leak_test/ext_state_leak_test_a.c -static
	gcc -o usr-bin-tests/ext_state_leak_test_b ext_state_leak_test/ext_state_leak_test_b.c -static
	gcc -o usr-bin-tests/ext_state_leak_test_c ext_state_leak_test/ext_state_leak_test_c.c -static
	echo "/usr/bin/tests/ext_state_leak_test_a" > usr-bin-tests/ext_state_leak_test_a.sh
	echo "/usr/bin/tests/ext_state_leak_test_b" > usr-bin-tests/ext_state_leak_test_b.sh
	echo "/usr/bin/tests/ext_state_leak_test_c" > usr-bin-tests/ext_state_leak_test_c.sh

prxcr0: prxcr0/prxcr0.c
	gcc -o usr-bin-tests/prxcr0 prxcr0/prxcr0.c -static

xstate_detect: xstate_detect/xstate_detect.c
	gcc -o usr-bin-tests/xstate_detect xstate_detect/xstate_detect.c -static -std=c99

xsaveopt_test: xsaveopt_test/xsaveopt_test.c
	gcc -o usr-bin-tests/xsaveopt_test xsaveopt_test/xsaveopt_test.c -static

ak-kfs: akaros/kern/kfs/ext_state_leak_test.sh
	cp akaros/kern/kfs/ext_state_leak_test.sh $(AKAROS)/kern/kfs/ext_state_leak_test.sh