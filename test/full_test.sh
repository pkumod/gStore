#! /bin/env bash

#TODO: why no size.log?? always error!!!

#in some system, maybe /usr/bin/ instead of /bin/
#according executables to deal with dbms
#NOTICE: require that virtuoso-openlink/openrdf-sesame/apache-jena is installed and gstore(single mode) is compiled!

#WARN:test gstore, jena, virtuoso with lubm, bsbm, watdiv, dbpedia
#when testing sesame and others, just use bsbm and watdiv because format in lubm
#i snot supported by sesame(invalid IRI), and dbpedia may be too large

#NOTICE+WARN:if only one db or no need to compare the database size, 
#then comment the removing db phrases for virtuoso!!!

#QUERY:do we need to empty the buffer after each dbms to compare the performance?
#query performance should focus on warm instead of cold

line1=--------------------------------------------------
line2=##################################################
#path=/media/data/
path=/home/data/

db0=${path}WatDiv/
db1=${path}LUBM/
db2=${path}DBpedia/
#db3=${path}BSBM/

#db=($db0 $db1 $db2 $db3)	#db[4]=db4
#db=($db0 $db1 $db2)	#db[4]=db4
db=(WatDiv/ LUBM/ BSBM/ DBpedia/)
#db=(WatDiv/ LUBM/)
#db=(TEST/)

#BETTER: add yago2/yago3, dblp...add more queries

length1=${#db[*]}		#or @ instead of *

#BETTER: let user indicate the executable directory
gstore=~/gStore/
virtuoso=~/virtuoso/
jena=~/jena/
gstore2=~/devGstore/

#NOTICE: maybe oldGstore and newGstore

#NOTICE:remove debug and use -o2 not -g when testing gStore

#dbms_path=($gstore $jena $sesame $virtuoso)
#dbms_name=(gstore jena sesame virtuoso)
#
#dbms_path=($gstore $jena $virtuoso)
#dbms_name=(gstore jena virtuoso)
#
#dbms_path=($gstore2 $gstore)
#dbms_name=(gstore gstore)
#
dbms_path=($gstore2)
dbms_name=(gstore)

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
		cntdbINFO=${cntdb}.info
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
			name=${dbms_name[j]}
			echo $name
			mkdir ${cntdb}
			mkdir ${cntdbINFO}

			#build logs structure
			echo "build logs structure!"
			if [ $dsnum -eq 1 ]
			then
				initial
			fi

			mkdir ${log1}/${cntdb}	#pass the cntdb if using function
			#touch ${log2}/${cntdb}.log

			if [ ${name}x = gstorex ]	#add a x in case of empty, otherwise will unary error
			#if [ ${j} -eq 0 ]	
			then
				echo "this is for gstore!"
				lcov -z -d ./
				bin/gbuild $cntdb $tmpdb > load.txt 2>&1
				gcov -a -b -c gbuild.cpp
				lcov --no-external --directory . --capture --output-file ${cntdb}/load.info
				#awk '{if($1=="after" && $2=="build," && $3=="used"){split($4, a,"m");print "time:\t"a[1]}}'  load.txt > load_${cntdb}.log
				awk '{if($1=="after" && $2=="build," && $3=="used"){split($4, a,"m");print "'$cntdb'""\t"a[1]}}'  load.txt >> ${log3}/time.log
				#elif [ ${dbms[j]}x = ${virtuoso}x ]
				#elif [ ${dbms[j]}x = ${sesame}x ]
				#elif [ ${dbms[j]}x = ${jena}x ]
			elif [ ${name}x = jenax ]
			#elif [ ${j} -eq 1 ]
			then
				echo "this is for jena!"
				bin/tdbloader --loc "$cntdb" "$tmpdb" > load.txt 2>&1
				#awk '{if(NR==1){s=$1}else{t=$1}}END{split(s,a,":");split(t,b,":");ans=0+(b[1]-a[1])*3600+(b[2]-a[2])*60+(b[3]-a[3]);printf("%s\t%d\n", "time:", ans*1000);}' load.txt > load_${cntdb}.log
				#NOTICE:if use more than one day, the time computed maybe <0
				awk '{if(NR==1){s=$1}else{t=$1}}END{split(s,a,":");split(t,b,":");ans=0+(b[1]-a[1])*3600+(b[2]-a[2])*60+(b[3]-a[3]);printf("%s\t%d\n", "'$cntdb'", ans*1000);}' load.txt >> ${log3}/time.log
				#cat load.txt >> "load_${cntdb}.log"
			elif [ ${name}x = sesamex ]
			#elif [ ${j} -eq 2 ]
			then
				#NOTICE+WARN:not suitable to lubm(format: not valid IRI)
				echo "this is for sesame!"
				#write instructions into run.sql
				>run.sql
				echo -e "create native\n${cntdb}\n${cntdb}\n\n\nopen ${cntdb}" >> run.sql
				echo "load ${tmpdb}" >> run.sql
				echo -e "close\nquit" >> run.sql
				bin/console.sh < run.sql > load.txt
				awk '{if($1=="Data" && $2 == "has" && $3 == "been"){split($8, a, "(");printf("%s\t%d\n", "'$cntdb'", a[2]);}}' load.txt >> ${log3}/time.log
			elif [ ${name}x = virtuosox ]
			#elif [ ${j} -eq 3 ]
			then
				echo "this is for virtuoso!"
				#maybe write instructions into run.sql
				#>run.sql
				#echo "ld_dir('${path}/${db[i]}/database/', '${cntdb}', '${cntdb}');" >> run.sql
				#echo "rdf_loader_run();" >> run.sql
				#echo "checkpoint;" >> run.sql
				>load.txt
				bin/isql 1111 dba dba exec="ld_dir('${path}/${db[i]}/database/', '${cntdb}', '${cntdb}');" | awk '{if($1=="Done."){print $3}}' >> load.txt
				bin/isql 1111 dba dba exec="rdf_loader_run();" | awk '{if($1=="Done."){print $3}}' >> load.txt
				bin/isql 1111 dba dba exec="checkpoint;" | awk '{if($1=="Done."){print $3}}' >> load.txt
				awk 'BEGIN{sum=0}{sum+=$0}END{printf("%s\t%d\n", "'$cntdb'", sum);}' load.txt >> ${log3}/time.log
			fi
			mv load.txt ${cntdbINFO}/

			#ls -l sums the actual size, unit is k
			echo "now to sum the database size!"
			#NOTICE:the unit is KB
			#ls -lR "$cntdb" | awk 'BEGIN{sum=0}{if($1=="total"){sum=sum+$2}}END{print "size:\t"sum}' >> load_${cntdb}.log
			#if [ ${j} -eq 3 ] #virtuoso
			if [ ${name}x = virtuosox ]
			then
				#NOTICE:this db also includes the initial data
				#realDB="../database/virtuoso.db"
				realDB="database/virtuoso.db"
				#the original size of virtuoso db is 39845888B, not so accurate
				ls -l "$realDB" | awk '{sum=$5/1000-39846;print "'$cntdb'""\t"sum}' >> ${log3}/size.log
			else
				if [ ${name}x = gstorex -o ${name}x = jenax ]
				#if [ ${j} -lt 2 ]
				then
					realDB="$cntdb"
				elif [ ${name}x = sesamex ]
				#elif [ ${j} -eq 2 ]
				then
					#NOTICE:not quoted the string!
					realDB=~/.aduna/openrdf-sesame-console/repositories/
					realDB=${realDB}${cntdb}
				fi
				lang=`echo $LANG`
				if [ $lang = $English ]
				then
					ls -lR "$realDB" | awk 'BEGIN{sum=0}{if($1=="total"){sum=sum+$2}}END{print "'$cntdb'""\t"sum}' >> ${log3}/size.log
				elif [ $lang = $Chinese ]
				then
					ls -lR "$realDB" | awk 'BEGIN{sum=0}{if($1=="总用量"){sum=sum+$2}}END{print "'$cntdb'""\t"sum}' >> ${log3}/size.log
				else
					echo "the language of the operation system is not supported!"
				fi
			fi

			timelog=${log2}/${cntdb}.log
			touch $timelog
			#NOTICE:we remove all duplicates to compare, due to different dbms preferences
			#For example, sesame and virtuoso will not include any duplicates
			for query in `ls ${path}/${db[i]}/query/*.sql`
			do
				#NOTICE:we expect there are no duplicates in sesame and virtuoso
				echo $query
				#build logs structure
				anslog=${log1}/${cntdb}/${query##*/}.log
				#touch $anslog	#needed because the result maybe empty
				>${anslog}
				if [ ${name}x = gstorex ]    #add a x in case of empty (need a space from ])
				#if [ ${j} -eq 0 ]	
				then
					echo "this is for gstore!"
					#NOTICE:we do not add the start time in gquery.cpp, and we expect other dbms will also deal it this way.
					lcov -z -d ./
					bin/gquery "$cntdb" $query > ans.txt 2>&1
					gcov -a -b -c gquery.cpp
					lcov --no-external --directory . --capture --output-file ${cntdb}/${query##*/}.info
					awk -F ':' 'BEGIN{query="'$query'"}{if($1=="Total time used"){split($2, a, "m");split(a[1],b," ");}}END{print query"\t"b[1]}' ans.txt >> $timelog
					#grep "Total time used:" ans.txt | grep -o "[0-9]*ms" >> ${log2}/${cntdb}.log
					awk -F ':' 'BEGIN{flag=0;old="[empty result]"}{if(flag==1 && $0 ~/^?/){flag=2}else if(flag==2){if($0 ~/^$/){flag=3}else if($0 != old){print $0;old=$0}}else if(flag == 0 && $1 ~/^final result/){flag=1}}' ans.txt > $anslog
					#awk 'BEGIN{flag=0}{if(flag==1){print $0}if($1 ~/^final$/){flag=1}}' ans.txt > ${log1}/${cntdb}/${query}.log
				elif [ ${name}x = jenax ]
				#elif [ ${j} -eq 1 ]
				then
					echo "this is for jena!"
					#NOTICE: for program output in stderr(no cache), deal like this
					#./tdbquery --repeat 1,1 --time --results TSV --loc "$cntdb" --query $query > ans.txt 2>&1 
					bin/tdbquery --time --results TSV --loc "$cntdb" --query $query > ans.txt 2>&1 
					#NOTICE: redirect in awk, and jena 
					#use old var to remove duplicates(expect duplicates to be all together)
					#awk 'BEGIN{old=""}{if(NR>1){if($1 ~/Time:/ && $3 ~/sec/){time=$2*1000;print "'$query'""\t"time >> "'$timelog'"}else if(!($0 ~/^$/) && $0 != old){print $0 >> "'$anslog'";old=$0}}}' ans.txt
					awk 'BEGIN{old=""}{if(NR>1){if($1 ~/Time:/ && $3 ~/sec/){time=$2*1000;print "'$query'""\t"time >> "'$timelog'"}else if(!($0 ~/^?/) && $0 != old){print $0 >> "'$anslog'";old=$0}}}' ans.txt
				elif [ ${name}x = sesamex ]
				#elif [ ${j} -eq 2 ]
				then
					echo "this is for sesame!"
					#write instructions into run.sql
					>run.sql
					echo "open ${cntdb}" >> run.sql
					#pre="sparql "
					str=`cat ${query}`
					ins="sparql "${str}
					#echo "${pre}${str}" >> run.sql
					echo ${ins} >> run.sql
					echo -e "close\nquit" >> run.sql
					bin/console.sh < run.sql > ans.txt
					#awk 'BEGIN{flag=0;}{
					#if($0 ~/^+/){flag++}
					#else if(flag==2){
						#if($NF=="|"){end=NF-1}else{end=NF}
							#for(i=2;i<=end;++i){split($i, s, "|");printf("%s", s[1]) >> "'$anslog'";
								#if(i<end){printf("\t") >> "'$anslog'";}}
									#printf("\n") >> "'$anslog'";}
								#else if(flag==3){flag++;split($3, s, "(");
									#print "'$query'""\t"s[2] >> "'$timelog'"}}' ans.txt
					awk -F '|' 'BEGIN{flag=0;}{
					if($0 ~/^+/){flag++;}
					else if(flag==2){
						for(i=2;i<NF;++i){
							num=split($i, s, " ");
							for(j=1;j<num;++j){printf("%s ", s[j]) >> "'$anslog'";}
							printf("%s", s[j]) >> "'$anslog'";
							if(i<NF-1){printf("\t") >> "'$anslog'";}
						}
						printf("\n") >> "'$anslog'";
					}
					else if(flag==3){flag++;split($0, a, " ");split(a[3], b, "(");print "'$query'""\t"b[2] >> "'$timelog'"}
					}' ans.txt
					#cat > wcgdscdc.txt
				elif [ ${name}x = virtuosox ]
				#elif [ ${j} -eq 3 ]
				then
					echo "this is for virtuoso!"
					ins=`cat ${query}`
					ins="sparql "${ins}
					echo $ins > tmp.txt
					str=`awk '{for(i=1;i<=NF;++i){if($i=="WHERE" || $i=="where"){printf("from <%s> %s ", "'$cntdb'", $i)}else{printf("%s ", $i)}}}' tmp.txt`
					rm -f tmp.txt
					bin/isql 1111 dba dba exec="${str};" > ans.txt
					#awk 'BEGIN{flag=0}{if($0 ~/^____/){flag=1}else if(flag==1 && $0 ~/^$/){flag=2}else if(flag==2){if($0 ~/^$/){flag=3}else{for(i=1;i<=NF;++i){if($i ~/^http:/){str="<"$i">";}else{str="\""$i"\"";}printf("%s", str) >> "'$anslog'";if(i<NF)printf("\t") >> "'$anslog'";}printf("\n") >> "'$anslog'";}}else if(flag==3){split($0, s, " ");print "'$query'""\t"s[4] >> "'$timelog'";}}' ans.txt
					awk -F ' [ \t]+' 'BEGIN{flag=0}{if($0 ~/^____/){flag=1}else if(flag==1 && $0 ~/^$/){flag=2}else if(flag==2){if($0 ~/^$/){flag=3}else{for(i=1;i<=NF;++i){printf("%s", $i) >> "'$anslog'";if(i<NF)printf("\t") >> "'$anslog'";}printf("\n") >> "'$anslog'";}}else if(flag==3){split($0, s, " ");print "'$query'""\t"s[4] >> "'$timelog'";}}' ans.txt
				fi
				mv ans.txt ${cntdbINFO}/${query##*/}.txt

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

			if [ ${name}x = gstorex ]    #add a x in case of empty (need a space from ])
			then
				#now to add the coverage info for this dataset
				str="lcov"
				for info in `ls *.info`
				do
					str="${str}"" -a ${info}"
				done
				str="${str}"" -o COVERAGE/${cntdb}.info"
				`${str}`
			fi

			#remove the db when finished
			echo "now to remove the cntdb!"
			if [ ${name}x = gstorex -o ${name}x = jenax ]
			#if [ ${j} -lt 2 ]
			then
				#rm -rf "$cntdb"
				echo "not remove db now"
			elif [ ${name}x = sesamex ]
			#elif [ ${j} -eq 2 ]
			then
				>run.sql
				echo "drop ${cntdb}" >> run.sql
				echo "yes" >> run.sql
				echo "quit" >> run.sql
				bin/console.sh < run.sql
				rm -f run.sql
			elif [ ${name}x = virtuosox ]
			#elif [ ${j} -eq 3 ]
			then
				echo "not remove db now"
				#bin/isql 1111 dba dba exec="sparql clear graph <${cntdb}>;"
				#bin/isql 1111 dba dba exec="checkpoint;"
				#bin/isql 1111 dba dba exec="sparql clear graph <${cntdb}>;"
				#bin/isql 1111 dba dba exec="checkpoint;"
				#bin/isql 1111 dba dba exec="sparql clear graph <${cntdb}>;"
				#bin/isql 1111 dba dba exec="checkpoint;"
				#bin/isql 1111 dba dba exec="sparql clear graph <${cntdb}>;"
				#bin/isql 1111 dba dba exec="checkpoint;"
				#bin/isql 1111 dba dba exec="sparql clear graph <${cntdb}>;"
				#bin/isql 1111 dba dba exec="checkpoint;"
				#bin/isql 1111 dba dba exec="sparql clear graph <${cntdb}>;"
				#bin/isql 1111 dba dba exec="checkpoint;"
				#bin/isql 1111 dba dba exec="sparql clear graph <${cntdb}>;"
				#bin/isql 1111 dba dba exec="checkpoint;"
				#bin/isql 1111 dba dba exec="sparql clear graph <${cntdb}>;"
				#bin/isql 1111 dba dba exec="checkpoint;"
				#bin/isql 1111 dba dba exec="sparql clear graph <${cntdb}>;"
				#bin/isql 1111 dba dba exec="checkpoint;"
				#bin/isql 1111 dba dba exec="sparql clear graph <${cntdb}>;"
				#bin/isql 1111 dba dba exec="checkpoint;"
				#bin/isql 1111 dba dba exec="delete from db.dba.load_list;"
			fi
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
					if [ ${dbms_name[p]}x = virtuosox -o ${dbms_name[q]}x = virtuosox ]
					#if [ $p -eq 3 -o $q -eq 3 ]
					then
					#WARN+NOTICE:the output in virtuoso is without '<>' or '""', so it is hard to compare!
					#url begins with "http://" is entities, but some others may also be, even like "FullProfessor0" in LUBM
					#So we print all 'Y' when encounter results from virtuoso
					#Another way is that we can remove all <> or "" and sort again, when comparing virtuoso and others
					#WARN:there seems to be other questions with the query result of virtuoso!
						if [ ${dbms_name[p]}x = virtuosox ]
						then
							x=$p
							y=$q
						else
							x=$q
							y=$p
						fi
						awk -F '\t' '{
						for(i=1; i<=NF; ++i){
							sub("^[<\"]", "", $i); sub("[>\"]$", "", $i); printf("%s", $i); 
							if(i<NF)printf("\t");}printf("\n");}' ${dbms_path[y]}/${tmplog} > change.txt.bak
						sort -t $'\t' -u change.txt.bak > change.txt
						diff ${dbms_path[x]}/${tmplog} change.txt > comp2.txt
						rm -f comp2.txt
						#cat > tmp.txt
						#awk -F '\t' 'BEGIN{flag=0}{
						#if(NR==FNR){map[NR]=$0}
						#else if(flag==0){
						#	num1=split(map[FNR],str1,"\t");
						#	num2=split($0,str2,"\t");
						#	if(num1 != num2){
						#		flag=1;
						#	}
						#	else{
						#		for(i=1;i<=num;++i){
						#			if(str1[i]!=str2[i]){
						#				flag=1;break}}}}}
						#			END{print "'${query##*/}'""\tY"}' ${dbms_path[p]}/${tmplog} ${dbms_path[q]}/${tmplog} >> compare.txt
					else
						diff ${dbms_path[p]}/${tmplog} ${dbms_path[q]}/${tmplog} > comp2.txt
						rm -f comp2.txt
						#NOTICE:the col num is almost all ok for query results
						#WARN:what if row num is different?
						#awk -F '\t' 'BEGIN{flag=0}{
						#if(NR==FNR){map[NR]=$0}
						#else if(flag==0){
						#	num1=split(map[FNR],str1,"\t");
						#	num2=split($0,str2,"\t");
						#	if(num1 != num2){
						#		flag=1;
						#	}
						#	else{
						#		for(i=1;i<=num1;++i){
						#			if(str1[i]!=str2[i]){
						#				flag=1;break}}}}}
						#			END{
						#				if(flag==0){print "'${query##*/}'""\tY"}
						#				else{print "'${query##*/}'""\tN"}}' ${dbms_path[p]}/${tmplog} ${dbms_path[q]}/${tmplog} >> compare.txt
					fi

					if [ $? -ne 0 ]
					then
						echo -e ${query##*/}"\tN" >> compare.txt
					else
						echo -e ${query##*/}"\tY" >> compare.txt
					fi
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
			}}' ${log3}/size.log ${tsv4}.bak > ${tsv4}
		rm -f ${tsv4}.bak
	fi
done


#generate coverage information view for gstore
cd ${gstore}
str="lcov"
for info in `ls COVERAGE/*.info`
do
	#info=${info##*/}
	str="${str}"" -a ${info}"
done
str="${str}"" -o COVERAGE/gstore.info"
`${str}`
lcov --remove COVERAGE/gstore.info 'Server/*' 'Main/*' 'Parser/*'
genhtml --output-directory COVERAGE --frames --show-details COVERAGE/gstore.info

echo "this is the end of full test!"
echo "please visit the result.log/, time.log/ and load.log/"
echo "you can use excel to load the .tsv files"

