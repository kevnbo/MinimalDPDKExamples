# https://github.com/NEOAdvancedTechnology/MinimalDPDKExamples.git

# note:
# this will take the eth1 (and eth2 if present) interfaces and attach them
# to dpdk ena drivers.  If things don't work it's harder to debug, so first
# check they have appropriate subnet connectivity via ping
#
echo 1024 > /sys/kernel/mm/hugepages/hugepages-2048kB/nr_hugepages
echo 0 > /proc/sys/kernel/randomize_va_space

modprobe uio
modprobe hwmon

# igb_uio is required for ena, note: it's not built by default for dpdk20
# try dpdk 19.05 old school make not meson
#
ifconfig eth1 down
ifconfig eth2 down
insmod ./igb_uio.ko

./dpdk-devbind.py --bind=igb_uio 00:06.0
./dpdk-devbind.py --bind=igb_uio 00:07.0
./dpdk-devbind.py --status

# to revert it,
# dpdk-devbind.py --bind=ena 00:006.0

