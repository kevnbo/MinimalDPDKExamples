#!/bin/sh

d=$(pwd)

sudo yum groupinstall "Development tools"

curl -o tf.zip https://releases.hashicorp.com/terraform/0.13.0/terraform_0.13.0_linux_amd64.zip
unzip tf.zip
./terraform init

cd ~/
git clone https://github.com/DPDK/dpdk.git
cd dpdk
git checkout v19.05
make defconfig && make

# for meson/ninja you need
# yum install python3-pip ninja-build
# but meson build doesn't build the necessary igb_uio kmod
# and doesn't work with the below w/o install

# go for coffee etc

cd $d
export RTE_SDK=$d
export RTE_TARGET=$d/build
for d in minimal_{rx,tx); do
	make
done

# check base thrpt with something like this
# should easily get 20Gbps with c5n
#
# sudo yum install iperf3
# 
# iperf -s
# iperf3 --parallel 2 -i 1 -V  -p 8000 -c 10.28.14.150




