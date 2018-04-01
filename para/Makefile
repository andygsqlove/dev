obj-m = para.o

kernel_dir=/home/andy/work/kernel/linux-2.6.35.7-gec-v3.0-gt110

all:
	$(MAKE) -C $(kernel_dir) M=$(shell pwd) modules
	