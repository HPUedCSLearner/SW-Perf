
if [ $# != 3 ]; then
    echo 'Usage: swaddr2line.sh executefile addressfile functionfile'
    exit
fi
 
EXECUTABLE="$1"
TRACELOG="$2"
OUTLOG="$3"

cat $2 | while read funaddr
		do
            echo $funaddr >> $3
            echo -n `swaddr2line -e $1 -f $funaddr` >> $3
            echo ' ' >> $3
		done
