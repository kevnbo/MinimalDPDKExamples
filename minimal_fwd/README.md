A simple l3-ish forwarder for trying in AWS

with 3 systems, L, M, R, M acts as a forwarder between L+R using DPDK.
M has 2 dpdk interfaces, eth1+eth2; L+R have 1 interface attached to the same traffic subnet with /24 cidr, e.g., 10.28.28.0/24.  L+R eth1 can be assgined to dpdk or linux depending on what you want to test.

to run:
```
sudo ./minimal_fwd -- -p 0x3 -a 212,175,12:a2:ad:7c:37:bf -a 131,38,12:56:ea:69:e8:0f
```

where `212` is ip address last byte assignedfor M.eth1; 175 is last byte ip L.eth1 and 12:a2,.. is mac of L.eth1
Similarly `131,38,12:56..` are IP/mac of `M.eth2`, `R.eth1`

Now L can ping R via M with `ping 10.28.28.212` and R can ping L via M with `ping 10.28.28.131`

Or iperf from L to R (R running iperf3 -s):
```
[ec2-user@ip-10-28-2-97 ~]$ iperf3 -u -b 1Gb -c 10.28.28.212
```

With `iperf iperf3 -l 9000 -P 5 -u -b 5Gb -c 10.28.28.212` it gets ~18Gb/s on c5nxlarge which should be capable of 25Gb/s before you play with other traffic generators.






