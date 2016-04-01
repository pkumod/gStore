#! /bin/env bash

#in some system, maybe /usr/bin/ instead of /bin/
#according executables to deal with dbms
#NOTICE: require that virtuoso/sesame/jena is installed and gstore is compiled!

line1=--------------------------------------------------
line2=##################################################
path=/media/data/
#db0=${path}WatDiv/
#db1=${path}LUBM/
#db2=${path}DBpedia/
#db3=${path}BSBM/
#db=($db0 $db1 $db2 $db3)	#db[4]=db4
db=(WatDiv/ LUBM/ BSBM/ DBpedia/)
#BETTER: add yago2/yago3, dblp...add more queries
length1=${#db[*]}		#or @ instead of *

#BETTER: let user indicate the executable directory
gstore=/home/zengli/devGstore/
virtuoso=/media/wip/zengli/virtuoso/bin/
sesame=/media/wip/zengli/sesame/bin/
jena=/media/wip/zengli/jena/bin/
#NOTICE: maybe oldGstore and newGstore
#dbms_path=($gstore $jena $sesame $virtuoso)
dbms_path=($gstore $jena)
dbms_name=(gstore jena)
#dbms_name=(gstore jena sesame virtuoso)
length2=${#dbms_path[*]}		#or @ instead of *

#the language of the current operation system
Chinese=zh_CN.utf8
English=en_US.utf8

#for each db, compare, pass db and query as parameter 
#firstly load database, then query with unique program
#output format:	in each dbms, time.log/ result.log/
#use each dataset name as subfolder like lubm_10.nt/ in result.log/
#and time.log/lubm_10.nt.log, and for each query corresponding
#to a dataset: result.log/lubm_10.nt/q1.sql.log
#Finally, in the directory where this script is placed in, also 
#build result.log/ and time.log/
#result.log/lubm_10.nt.tsv time.log/lubm_10.nt.tsv size.log.tsv

#below is old:
#time log should be used in excel, and compare result log:
#diff or grep -vFf file1 file2
#better to compare line by line using awk

log1=result.log/
log2=time.log/
log3=load.log/	

#clean logs in home(this script)
home=`pwd`
if [ -d ${home}/garbage/ ]	#! as not
then
	rm -rf ${home}/garbage/
fi
mkdir ${home}/garbage/
if [ -d ${home}/${log1} ]
then 
	rm -rf ${home}/${log1}
fi
mkdir ${home}/${log1}
if [ -d ${home}/${log2} ]
then
	rm -rf ${home}/${log2}
fi
mkdir ${home}/${log2}
if [ -d ${home}/${log3} ]
then
	rm -rf ${home}/${log3}
fi
mkdir ${home}/${log3}

#clean logs in each dbms
function initial()
{
	if [ -d $log1 ]
	then
		rm -rf $log1
	fi
	mkdir $log1
	if [ -d $log2 ]
	then
		rm -rf $log2
	fi
	mkdir $log2
	if [ -d $log3 ]
	then
		rm -rf $log3
	fi
	mkdir $log3
}

#size.tsv:the size after loaded		time.tsv:time used to load
tsv3=${home}/${log3}time.tsv
tsv4=${home}/${log3}size.tsv
dsnum=0
for i in `seq $length1`
do
	i=`expr $i - 1`
	for tmpdb in `ls ${path}/${db[i]}/database/*.nt`
	do
		dsnum=`expr $dsnum + 1`
		if [ $dsnum -ne 1 ]
		then
			sleep 60	#for other processes
			#sudo echo 3 > /proc/sys/vm/drop_caches
		fi
		cntdb="${tmpdb##*/}"
		echo "$tmpdb"	#in case of special characters like &
		tsv1=${home}/${log1}/${cntdb}.tsv	#compare result
		tsv2=${home}/${log2}/${cntdb}.tsv	#compare time
		echo $tsv1
		echo $tsv2
		#load this database into each dbms
		for j in `seq $length2`
		do
			j=`expr $j - 1`
			cd ${dbms_path[j]}
			#build logs structure
			echo "build logs structure!"
			if [ $dsnum -eq 1 ]
			then
				initial
			fi
			mkdir ${log1}/${cntdb}	#pass the cntdb if using function
			#touch ${log2}/${cntdb}.log
			#if [ ${dbms_name[j]}x = ${gstore}x ]	#add a x in case of empty 
			if [ ${j} -eq 0 ]	#otherwise will unary error
			then
				echo "this is for gstore!"
				bin/gload $cntdb $tmpdb > load.txt
				#awk '{if($1=="after" && $2=="build," && $3=="used"){split($4, a,"m");print "time:\t"a[1]}}'  load.txt > load_${cntdb}.log
				awk '{if($1=="after" && $2=="build," && $3=="used"){split($4, a,"m");print "'$cntdb'""\t"a[1]}}'  load.txt >> ${log3}/time.log
				#elif [ ${dbms[j]}x = ${virtuoso}x ]
				#elif [ ${dbms[j]}x = ${sesame}x ]
				#elif [ ${dbms[j]}x = ${jena}x ]
			elif [ ${j} -eq 1 ]
			then
				echo "this is for jena!"
				./tdbloader --loc "$cntdb" "$tmpdb" > load.txt 2>&1
				#awk '{if(NR==1){s=$1}else{t=$1}}END{split(s,a,":");split(t,b,":");ans=0+(b[1]-a[1])*3600+(b[2]-a[2])*60+(b[3]-a[3]);printf("%s\t%d\n", "time:", ans*1000);}' load.txt > load_${cntdb}.log
				awk '{if(NR==1){s=$1}else{t=$1}}END{split(s,a,":");split(t,b,":");ans=0+(b[1]-a[1])*3600+(b[2]-a[2])*60+(b[3]-a[3]);printf("%s\t%d\n", "'$cntdb'", ans*1000);}' load.txt >> ${log3}/time.log
				#cat load.txt >> "load_${cntdb}.log"
			elif [ ${j} -eq 2 ]
			then
				echo "this is for sesame!"
				#TODO
			elif [ ${j} -eq 3 ]
			then
				echo "this is for virtuoso!"
				#TODO
			fi
			#ls -l sums the actual size, unit is k
			echo "now to sum the database size!"
			#ls -lR "$cntdb" | awk 'BEGIN{sum=0}{if($1=="total"){sum=sum+$2}}END{print "size:\t"sum}' >> load_${cntdb}.log
			lang=`echo $LANG`
			if [ $lang = $English ]
			then
				ls -lR "$cntdb" | awk 'BEGIN{sum=0}{if($1=="total"){sum=sum+$2}}END{print "'$cntdb'""\t"sum}' >> ${log3}/size.log
			elif [ $lang = $Chinese ]
			then
				ls -lR "$cntdb" | awk 'BEGIN{sum=0}{if($1=="总用量"){sum=sum+$2}}END{print "'$cntdb'""\t"sum}' >> ${log3}/size.log
			else
				echo "the language of the operation system is not supported!"
			fi

			timelog=${log2}/${cntdb}.log
			touch $timelog
			for query in `ls ${path}/${db[i]}/query/*.sql`
			do
				echo $query
				#build logs structure
				anslog=${log1}/${cntdb}/${query##*/}.log
				touch $anslog	#needed because the result maybe empty
				if [ ${j} -eq 0 ]	#add a x in case of empty 
				then
					echo "this is for gstore!"
					bin/gquery "$cntdb" $query > ans.txt
					awk -F ':' 'BEGIN{query="'$query'"}{if($1=="Total time used"){split($2, a, "m");split(a[1],b," ");}}END{print query"\t"b[1]}' ans.txt >> $timelog
					#grep "Total time used:" ans.txt | grep -o "[0-9]*ms" >> ${log2}/${cntdb}.log
					awk -F ':' 'BEGIN{flag=0;old=""}{if(flag==1 && $0 ~/^$/){flag=2}if(flag==1 && !($0 ~/[empty result]/) && $0 != old){print $0;old=$0}if($1=="final result is"){flag=1}}' ans.txt > $anslog
					#awk 'BEGIN{flag=0}{if(flag==1){print $0}if($1 ~/^final$/){flag=1}}' ans.txt > ${log1}/${cntdb}/${query}.log
				elif [ ${j} -eq 1 ]
				then
					echo "this is for jena!"
					#NOTICE: for program output in stderr(no cache), deal like this
					./tdbquery --repeat 2,1 --time --results TSV --loc "$cntdb" --query $query > ans.txt 2>&1 
					#NOTICE: redirect in awk, and jena 
					#use old var to remove duplicates
					awk 'BEGIN{old=""}{if(NR>1){if($1 ~/Time:/ && $3 ~/sec/){time=$2*1000;print "'$query'""\t"time >> "'$timelog'"}else if(!($0 ~/^$/) && $0 != old){print $0 >> "'$anslog'";old=$0}}}'	ans.txt
				elif [ ${j} -eq 2 ]
				then
					echo "this is for sesame!"
					#TODO
				elif [ ${j} -eq 3 ]
				then
					echo "this is for virtuoso!"
					#TODO
				fi
				#NOTICE:the same record should be placed together before sorting!
				#sort according to the path order
				echo "now to sort anslog!"
				mv $anslog ${anslog}.bak
				#use own defined select-sort function
				#this function can also delete duplicates
				#BETTER: must use external-sorting when too large
			#	awk -F '\t' '{for(i=1;i<=NF;++i)arr[NR]=$0}
			#	END{
			#		nr=sortArr(arr,NR,NF);
			#		for(i=1;i<=nr;++i){print arr[i]}}
			#	function sortArr(arr, nr, nf){
			#		for(p=1;p<nr;++p){
			#			min=p;
			#			for(q=p+1;q<=nr;++q){
			#				ret=less(arr[q],arr[min],nf);
			#				if(ret==1){min=q}
			#				else if(ret==0){swap(arr,q,nr);nr=nr-1;q=q-1}}
			#			if(min!=p){swap(arr,p,min)}}
			#		return nr}
			#	function swap(arr,t1,t2){t=arr[t1];arr[t1]=arr[t2];arr[t2]=t}
			#	function less(t1,t2,nf){
			#		split(t1,s1,"\t");
			#		split(t2,s2,"\t");
			#		for(k=1;k<=nf;++k){
			#			print s1[k]"\t"s2[k]
			#			if(s1[k]<s2[k]){ return 1 }
			#			else if(s1[k]>s2[k]) { return 2 }
			#			else { continue; } }
			#		return 0 }' ${anslog}.bak > ${anslog}
				#-k1n -k2r ...
				sort -t $'\t' -u ${anslog}.bak > ${anslog}
				rm -f ${anslog}.bak
			done
			echo "now to sort timelog!"
			mv $timelog ${timelog}.bak
			awk -F '\t' '{print $1"\t"$2 | "sort -k1"}' ${timelog}.bak > ${timelog}
			rm -f ${timelog}.bak
			#remove the db when finished
			echo "now to remove the cntdb!"
			rm -rf "$cntdb"
			#BETTER:remove *.txt in each dbms path
			#rm -f *.txt
			#compare time and construct the TSV table
			if [ ${j} -eq 0 ]
			then
				echo "this is the first dbms!"
				awk -F '\t' 'BEGIN{
					print "Time\t""'${dbms_name[j]}'" }
				{ num=split($1,str,"/"); print str[num]"\t"$2 }' ${timelog} > ${tsv2}
			else
				echo "this is not the first dbms!"
				mv ${tsv2} ${tsv2}.bak
				awk -F '\t' '{
				if(NR==FNR) {
					num=split($1,str,"/");
					map[str[num]]=$2 }
				else {
					if(FNR==1) { print $0"\t""'${dbms_name[j]}'" }
					else { print $0"\t"map[$1] }
				}}' ${timelog} ${tsv2}.bak > ${tsv2}
				rm -f ${tsv2}.bak
			fi
		done
		#compare the result and construct the TSV table
		echo "now to compare the results!"
		cd ${home}
		tvar1=`expr $length2 - 1`  
		tvar2=`expr $length2 - 2`
		for p in `seq 0 $tvar2`
		do
			tvar3=`expr $p + 1`
			for q in `seq $tvar3 $tvar1`
			do
				echo $p,$q
				>compare.txt
				for query in `ls ${path}/${db[i]}/query/*.sql`
				do
					echo "compare: " $query
					tmplog=${log1}/${cntdb}/${query##*/}.log
					awk -F '\t' 'BEGIN{flag=0}{
					if(NR==FNR){map[NR]=$0}
					else if(flag==0){
						num=split(map[FNR],str1,"\t");
						split($0,str2,"\t");
						for(i=1;i<=num;++i){
							if(str1[i]!=str2[i]){
								flag=1;break}}}}
								END{
								if(flag==0){print "'${query##*/}'""\tY"}
								else{print "'${query##*/}'""\tN"}}' ${dbms_path[p]}/${tmplog} ${dbms_path[q]}/${tmplog} >> compare.txt
					#diff ${dbms_path[p]}/${tmplog} ${dbms_path[q]}/${tmplog}
					#if [ $? -ne 0 ]
					#then
					#	echo -e ${query##*/}"\tN" >> compare.txt
					#else
					#	echo -e ${query##*/}"\tY" >> compare.txt
					#fi
				done
				echo "all queries done!"
				name=${dbms_name[p]}_${dbms_name[q]}
				if [ $p -eq 0 ] && [ $q -eq 1 ]
				then
					awk -F '\t' 'BEGIN{print "Result\t""'$name'"}{print $0}' compare.txt > ${tsv1}
				else
					mv ${tsv1} ${tsv1}.bak
					awk -F '\t' '{
					if(NR==FNR) { map[$1]=$2 }
					else {
						if(FNR==1) { print $0"\t""'$name'" }
						else { print $0"\t"map[$1] }
					}}' compare.txt ${tsv1}.bak > ${tsv1}
					rm -f ${tsv1}.bak
				fi
			done
		done
	done
done

#build the load.log/ in home(this script)
echo "now to build the load.log!"
for j in `seq $length2`
do
	j=`expr $j - 1`
	cd ${dbms_path[j]}
	if [ $j -eq 0 ]
	then
		echo "this is the first dbms!"
		awk -F '\t' 'BEGIN{print "dataset\\dbms\t""'${dbms_name[j]}'"}{print $0}' ${log3}/time.log > $tsv3
		awk -F '\t' 'BEGIN{print "dataset\\dbms\t""'${dbms_name[j]}'"}{print $0}' ${log3}/size.log > $tsv4
	else
		echo "this is not the first dbms!"
		mv ${tsv3} ${tsv3}.bak
		awk -F '\t' '{
		if(NR==FNR) { map[$1]=$2 }
		else {
			if(FNR==1) { print $0"\t""'${dbms_name[j]}'" }
			else { print $0"\t"map[$1] }
			}}' ${log3}/time.log ${tsv3}.bak > ${tsv3}
		rm -f ${tsv3}.bak
		mv ${tsv4} ${tsv4}.bak
		awk -F '\t' '{
		if(NR==FNR) { map[$1]=$2 }
		else {
			if(FNR==1) { print $0"\t""'${dbms_name[j]}'" }
			else { print $0"\t"map[$1] }
			}}' ${log3}/time.log ${tsv4}.bak > ${tsv4}
		rm -f ${tsv4}.bak
	fi
done

echo "this is the end of full test!"
echo "please visit the result.log/, time.log/ and load.log/"
echo "you can use excel to load the .tsv files"

