bin/gload dbpedia.db /home/data/DBpedia/database/dbpedia2014.nt > load.txt 2>&1
echo "load finished"

bin/gsub dbpedia.db ../dbpedia_small > sub.txt 2>&1
echo "subed"

bin/gquery dbpedia.db /home/data/DBpedia/query/q0.sql > q0.txt 2>&1
echo "q0.sql finished"
bin/gquery dbpedia.db /home/data/DBpedia/query/q1.sql > q1.txt 2>&1
echo "q1.sql finished"
bin/gquery dbpedia.db /home/data/DBpedia/query/q2.sql > q2.txt 2>&1
echo "q2.sql finished"
bin/gquery dbpedia.db /home/data/DBpedia/query/q3.sql > q3.txt 2>&1
echo "q3.sql finished"
bin/gquery dbpedia.db /home/data/DBpedia/query/q4.sql > q4.txt 2>&1
echo "q4.sql finished"
bin/gquery dbpedia.db /home/data/DBpedia/query/q5.sql > q5.txt 2>&1
echo "q5.sql finished"

bin/gadd dbpedia.db ../dbpedia_small > add.txt 2>&1
echo "added"

echo "big tested"

