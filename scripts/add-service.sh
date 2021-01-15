#!bin/bash
echo -e "\033[43;35m run this script in bash using root priviledge \033[0m \n"

BASEDIR=$(pwd)
DIR=$BASEDIR/autorun-ghttp.sh
echo "export CXX=/usr/local/bin/gcc">$DIR
echo "export LD_LIBRARY_PATH=/usr/local/lib:/usr/local/lib64:\$LD_LIBRARY_PATH">>$DIR
echo "cd $BASEDIR/..">>$DIR
echo "bin/ghttp $1 &">>$DIR
echo $DIR>>/etc/rc.local
chmod +x $DIR
chmod +x /etc/rc.local
