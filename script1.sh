echo "Connect server to port 22222"
sleep 1
sock :22222 << END
PEERS?\n
GOSSIP:blablabla:12-23
4:Fragmented message to server%
GOSSIP:blabla:33:Concatinated message to Server%GOSSIP:blaaabla:3-23-3:Works!!%
GOSSIP:mBHL7IKilvdcOFKR03ASvBNX//ypQkTRUvilYmB1/OY=:2018-01-09-16-18-20-001Z:Tom eats Jerry%
GOS
SIP:b
labla:3
3:Duplicate me
ssage%
PEER:Alex:PORT=234:IP=12.34.24.23%
PEERS?\n
PEER:Klaus:POR
T=100:IP=13.24.32.10%
PEERS?\n
PEER:Klaus:PORT=12346:IP=127.0.0.1%
PEER:John:PORT=2356:IP=163.118.239.68%
PEERS?\n
PEER:Peter:PORT=1234:IP=172.0.0.1%
PEER:Peter:PORT=1234:IP=172.0.0.2%
PEER:Peter:PORT=1234:IP=172.0.0.3%
PEERS?\n
END
