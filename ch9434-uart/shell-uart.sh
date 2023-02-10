

#!/bin/bash
dataheader=$1
baudrate=$2

if [ $# -ne 2 ];then
	echo $0 /dev/ttyWCH0 115200
	exit
fi

data=1
while :
do
	
	senddata=$dataheader+$data;
#	echo $senddata;
	./test $1 $2 $senddata
	let data=data+1 
	
	if [ $data -gt 1000 ]; then
		let data=0;
	fi
	
	sleep 5;
done
