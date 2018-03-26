#create a folder testFolder in your code01 folder before running the server
gcc server.c -o server -lpthread
mkdir -p "testFolder/"
./server -p $1 -d "testFolder/"
