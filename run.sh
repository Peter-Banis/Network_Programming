#create a folder testFolder in your code01 folder before running the server
./compile.sh
mkdir -p "testFolder/"
./server -p $1 -d "testFolder/"
