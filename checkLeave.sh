./compileSC.sh
./server -p 12344 -d "test/" -c 12347 -s "127.0.0.1" << STOP
PEER:Klaus:PORT=5005:IP=127.0.0.1%
quit
STOP
sleep 2
./server -p 12345 -d "test/" -c 12347 -s "127.0.0.1" << END
Hello Server
quit
END
sleep 7
./server -p 12346 -d "test/" -c 12347 -s "127.0.0.1" << HALT
Still alive?
quit
HALT
