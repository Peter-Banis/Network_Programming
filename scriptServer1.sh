gcc server.c -o server -lpthread
mkdir -p "testFolder/"
./server -p 12346 -d "testFolder/"
