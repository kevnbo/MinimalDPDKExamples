ips=$*
#"10.28.1.226" # 10.28.1.79 10.28.1.131"

test -f addr.sh && . addr.sh

dpdk=$HOME/dpdk

for f in $ips; do 
	d="ec2-user@${f}"
	scp $dpdk/build/kmod/igb_uio.ko $dpdk/usertools/dpdk-devbind.py ./minimal_rx/build/minimal_rx $d:
	scp ./minimal_tx/build/minimal_tx ./setup.sh $d:
	scp ./minimal_fwd/build/minimal_fwd $d:
#	ssh $d sudo bash -x ./setup.sh
done

