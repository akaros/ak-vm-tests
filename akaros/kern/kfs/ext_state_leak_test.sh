#!/bin/ash

#
# Copyright 2016 Google Inc.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#

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
# pthread_test num_threads num_loops [num_vcores]
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

echo "Initial provision (mode 0) (prov -s):"
prov -s

# 4. Let the VMs run long enough to boot and set up their test programs (usually about 14sec)
usleep 20000000 # 20 sec

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
    echo "Allocations (prov -s):"
    prov -s
    usleep 10000000 # 10 seconds
    echo Completed swap-and-runs: $SWAPCT
done

kill $PID_PTH
kill $PID_A
kill $PID_B

echo "Finished extended state leak test."
