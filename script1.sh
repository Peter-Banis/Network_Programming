echo "Connect server to port 22222"
sleep 1
sock :22222 << END
PEERS?
PEER:Peter:PORT=12345:IP=172.0.0.1%
PEER:Peter:PORT=12345:IP=172.0.0.2%
PEER:Peter:PORT=12345:IP=172.0.0.3%
GOSSIP:mBHL7IKilvdcfFKR03ASvBNX//ypQkTRUvilYmB1/OY=:2018-01-09-16-
18-20-001Z:Fragmented message to server%
PEER:Klaus:PORT=12346:IP=127.0.0.1%PEERS?
GOSSIP:mBHL7IKilvdcOFKR03ASvBNX//ypQkTRUvilYmB1/OY4:2018-01-09-16-18-20-001Z:Concatinated message to Server%GOSSIP:mBHL7IKilvdcOFKR03ASvBNs//ypQkTRUvilYmB1/OY4:2018-01-09-16-18-20-001Z:Works!!%
GOSSIP:mBHL7IKilvdcOFKR03ASvBNX//ypQkTRUvilYmB1/OY=:2018-01-09-16-18-20-001Z:Tom eats Jerry%
GOSSIP:mBHL7IKilvdcOFKR03ASvBNX//ypQkTRUvilYmB1/OY=:2018-01-09-16-18-20-001Z:Tom eats Jerry%
GOS
SIP:mBHL7IoilvdcOFKR03ASvBNX
//ypQkTRUvilYmB1/OY=:2018-01-09-
16-18-20-001Z:Fragmented
message%
GOSSIP:mBHL7IKilvdcOFKR03ASvsNX//ypQkTRUvilYmB1/OY-:2018-01-09-16-18-20-001Z:Nearly completed%
PEERS?PEER:Alex:PORT=1234:IP=12.34.24.23%PEERS?
GOSSIP:mBHL7IKilvdcOFKR03ASvBNX//ypQkTRevil0mB1/OY=:2018-01-09-16-18-20-001Z:Bye Bye%
END
