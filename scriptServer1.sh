./compileSC.sh
mkdir -p "test/"
./server -p 12347 -d "test/" -c 12347 -s "127.0.0.1" -D 5
