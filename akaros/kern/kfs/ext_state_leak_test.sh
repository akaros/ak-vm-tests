#!/bin/ash

# can use pip pid from the monitor to see vcore allocation
# what about provisioning?

if [ $# -lt 1 ]
then
    echo Usage: $0 NUMSWAPS
    exit
fi

NUMSWAPS=$1

# 1. Provision the entire machine to pthread_test

# max_vcores returns the max number of vcores for the machine
max_vcores
MAXVC=$?

# pthread_test exists to hog the machine
# TODO: Need to figure out what these args to pthread_test do
# TODO: Do I need to run prov for pthread_test?
pthread_test 100 999999999 $MAXVC >> tmpfile 2>&1 &
PID_PTH=$!
echo "Launched pth_test, pid: $PID_PTH"

# prov:
# -tc means resource type = cores
# -p means PID
# -m means max amount of resources of given type get provisioned
# TODO: do I need to redirect prov's output to a file? other script uses tempfile
prov -tc -p$PID_PTH -m

# 2. Create two VMs. Neither will start running because there are no cores available.
vmrunkernel -c ext_state_leak_test_a xxx > ext_state_vm_a.out 2>&1 &
PID_A=$!
vmrunkernel -c ext_state_leak_test_b xxx > ext_state_vm_b.out 2>&1 &
PID_B=$!

echo Launched VM a and b, pids: a: $PID_A, b: $PID_B

# 3. Then provision 4 cores to each VM. (e.g. 1234 for VM1, 5678 for VM2)
prov -tc -p$PID_A -v1
prov -tc -p$PID_A -v2
prov -tc -p$PID_A -v3
prov -tc -p$PID_A -v4

prov -tc -p$PID_B -v5
prov -tc -p$PID_B -v6
prov -tc -p$PID_B -v7
prov -tc -p$PID_B -v8

echo "Initial provision (mode 0)"

# 4. Let the VMs run for a bit
usleep 100 # 100 microseconds

echo "Completed initial run (mode 0)"

MODE=0
SWAPCT=0
while [ $SWAPCT -lt $NUMSWAPS ]
do
    # 5. Provision the entire machine back to pthread_test
    prov -tc -p$PID_PTH -m

    # 6. Provision 4 cores to each VM, but this time swap the cores between them
    if [ $MODE -eq 1 ];
    then
        prov -tc -p$PID_A -v1
        prov -tc -p$PID_A -v2
        prov -tc -p$PID_A -v3
        prov -tc -p$PID_A -v4

        prov -tc -p$PID_B -v5
        prov -tc -p$PID_B -v6
        prov -tc -p$PID_B -v7
        prov -tc -p$PID_B -v8
        MODE=0
    else
        prov -tc -p$PID_A -v5
        prov -tc -p$PID_A -v6
        prov -tc -p$PID_A -v7
        prov -tc -p$PID_A -v8

        prov -tc -p$PID_B -v1
        prov -tc -p$PID_B -v2
        prov -tc -p$PID_B -v3
        prov -tc -p$PID_B -v4
        MODE=1
    fi
    SWAPCT=$((SWAPCT+1))

    echo Running after swap $SWAPCT to mode $MODE
    usleep 100
    echo Completed swap-and-runs: $SWAPCT
done

kill $PID_PTH
kill $PID_A
kill $PID_B

echo "Finished extended state leak test."
