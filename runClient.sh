javac Client.java
if [ $# -eq 4 ]
then
    java Client -p $1 -s $2 -m "$3" -t $4
elif [ $# -eq 3 ]
then
    java Client -p $1 -s $2 -m "$3"
else
    java Client -p $1 -s $2
fi
