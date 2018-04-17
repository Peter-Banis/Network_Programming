./compileSC.sh
./server -p 21212 -d "" -c 23232 -s "127.0.0.1" -m "Hello" << STOP
Hello server
Looks like he did :)
PEER:Spy Server:PORT=12345:IP=127.0.0.1%
PEERS?
Ohh! Hi there new server.
Adding some peers...
PEER:Klaus:PORT=12341:IP=127.0.0.1%
PEER:Peter:PORT=12342:IP=127.0.0.1%
Lets see how many peers we have now....
PEERS?
PEER:Klaus:PORT=12342:IP=127.0.0.2%
PEER:Poo:PORT=12342:IP=127.0.0.1%
PEER:Bear:PORT=12342:IP=127.0.0.1%
PEERS?
Bye Bye
Scrpit finished
quit
STOP
