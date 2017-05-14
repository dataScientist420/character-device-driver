#!/bin/bash

# compile kernel module
make clean
make

# If compilation has produced a loadable kernel module
if [ -f kernel_src/chardriver.ko ]
then
	# if the kernel module is already installed  
	lsmod | grep chardriver >& /dev/null
	if [ $? -eq 0 ]
	then
		# uninstall the existing kernel module  
    	sudo rmmod chardriver
	fi

	# install the new version of the kernel module
	sudo insmod kernel_src/chardriver.ko

	# remove user's executable if it exists
	[ -x user_src/main ] && rm user_src/main
	
	# compile and execute the user code which uses the module
	gcc user_src/main.c -o user_src/main
	./user_src/main
fi