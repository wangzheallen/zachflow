#!/bin/bash
PATH=/bin:/sbin:/usr/sbin:/usr/local/bin:~/bin:/usr/lib/lightdm/lightdm:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games:/home/jerom/bash/
export PATH
echo -e "hello world! \a \n"

x=0
for file in ~/newFlow/*.avi
do
echo "file:$file"
#bsnm = "${file%.avi}"
p=`echo $file|awk -F/ '{print $5}'`
echo "p:$p"
x=$(($x+1))
./newFlow -f $file -x ~/newFlow/x/$p -y ~/newFlow/y/$p -i ~/newFlow/i/$p 
done
exit 0;

