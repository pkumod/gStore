# make sure there exists directory DBPEDIA.info.info

bin/gbuild dbpedia /home/data/DBpedia/database/dbpedia170M.nt > DBPEDIA.info/load.txt 2>&1
echo "load finished"

bin/gquery dbpedia /home/data/DBpedia/query/q0.sql > DBPEDIA.info/q0.txt 2>&1
echo "q0.sql finished"
bin/gquery dbpedia /home/data/DBpedia/query/q1.sql > DBPEDIA.info/q1.txt 2>&1
echo "q1.sql finished"
bin/gquery dbpedia /home/data/DBpedia/query/q2.sql > DBPEDIA.info/q2.txt 2>&1
echo "q2.sql finished"
bin/gquery dbpedia /home/data/DBpedia/query/q3.sql > DBPEDIA.info/q3.txt 2>&1
echo "q3.sql finished"
bin/gquery dbpedia /home/data/DBpedia/query/q4.sql > DBPEDIA.info/q4.txt 2>&1
echo "q4.sql finished"
bin/gquery dbpedia /home/data/DBpedia/query/q5.sql > DBPEDIA.info/q5.txt 2>&1
echo "q5.sql finished"
bin/gquery dbpedia /home/data/DBpedia/query/q6.sql > DBPEDIA.info/q6.txt 2>&1
echo "q6.sql finished"

echo "dbpedia tested"

