bin/gload dbpedia.db /home/data/DBpedia/database/dbpedia2014.nt > DBPEDIA/load.txt 2>&1
echo "load finished"

bin/gquery dbpedia.db /home/data/DBpedia/query/q0.sql > DBPEDIA/q0.txt 2>&1
echo "q0.sql finished"
bin/gquery dbpedia.db /home/data/DBpedia/query/q1.sql > DBPEDIA/q1.txt 2>&1
echo "q1.sql finished"
bin/gquery dbpedia.db /home/data/DBpedia/query/q2.sql > DBPEDIA/q2.txt 2>&1
echo "q2.sql finished"
bin/gquery dbpedia.db /home/data/DBpedia/query/q3.sql > DBPEDIA/q3.txt 2>&1
echo "q3.sql finished"
bin/gquery dbpedia.db /home/data/DBpedia/query/q4.sql > DBPEDIA/q4.txt 2>&1
echo "q4.sql finished"
bin/gquery dbpedia.db /home/data/DBpedia/query/q5.sql > DBPEDIA/q5.txt 2>&1
echo "q5.sql finished"
#bin/gquery dbpedia.db /home/data/DBpedia/query/q6.sql > DBPEDIA/q6.txt 2>&1
#echo "q6.sql finished"

echo "dbpedia tested"

