#!bin/bash
echo -e "\033[43;35m run this script in bash using root priviledge \033[0m \n"

BASEDIR=$(pwd)
DIR=$BASEDIR/autorun-ghttp.sh
echo "cd $BASEDIR/..">$DIR
echo "bin/ghttp 9000 &">>$DIR
echo $DIR>>/etc/rc.local
chmod +x $DIR
chmod +x /etc/rc.local
