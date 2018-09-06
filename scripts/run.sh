bin/gload watdiv1000.db /home/data/WatDiv/database/watdiv_1000.nt > load.txt 2>&1
echo "load finished"

bin/gsub watdiv1000.db ../WATDIV1000_small > sub.txt 2>&1
echo "subed"

bin/gquery watdiv1000.db /home/data/WatDiv/query/C1.sql > C1.txt 2>&1
echo "C1.sql finished"
bin/gquery watdiv1000.db /home/data/WatDiv/query/C2.sql > C2.txt 2>&1
echo "C2.sql finished"
bin/gquery watdiv1000.db /home/data/WatDiv/query/C3.sql > C3.txt 2>&1
echo "C3.sql finished"
bin/gquery watdiv1000.db /home/data/WatDiv/query/F1.sql > F1.txt 2>&1
echo "F1.sql finished"
bin/gquery watdiv1000.db /home/data/WatDiv/query/F2.sql > F2.txt 2>&1
echo "F2.sql finished"
bin/gquery watdiv1000.db /home/data/WatDiv/query/F3.sql > F3.txt 2>&1
echo "F3.sql finished"
bin/gquery watdiv1000.db /home/data/WatDiv/query/L1.sql > L1.txt 2>&1
echo "L1.sql finished"
bin/gquery watdiv1000.db /home/data/WatDiv/query/L2.sql > L2.txt 2>&1
echo "L2.sql finished"
bin/gquery watdiv1000.db /home/data/WatDiv/query/L3.sql > L3.txt 2>&1
echo "L3.sql finished"
bin/gquery watdiv1000.db /home/data/WatDiv/query/S1.sql > S1.txt 2>&1
echo "S1.sql finished"
bin/gquery watdiv1000.db /home/data/WatDiv/query/S2.sql > S2.txt 2>&1
echo "S2.sql finished"
bin/gquery watdiv1000.db /home/data/WatDiv/query/S3.sql > S3.txt 2>&1
echo "S3.sql finished"

bin/gadd watdiv1000.db ../WATDIV1000_small > add.txt 2>&1
echo "added"

bin/gquery watdiv1000.db /home/data/WatDiv/query/C1.sql > C1.txt.bak 2>&1
echo "C1.sql finished"
bin/gquery watdiv1000.db /home/data/WatDiv/query/C2.sql > C2.txt.bak 2>&1
echo "C2.sql finished"
bin/gquery watdiv1000.db /home/data/WatDiv/query/C3.sql > C3.txt.bak 2>&1
echo "C3.sql finished"
bin/gquery watdiv1000.db /home/data/WatDiv/query/F1.sql > F1.txt.bak 2>&1
echo "F1.sql finished"
bin/gquery watdiv1000.db /home/data/WatDiv/query/F2.sql > F2.txt.bak 2>&1
echo "F2.sql finished"
bin/gquery watdiv1000.db /home/data/WatDiv/query/F3.sql > F3.txt.bak 2>&1
echo "F3.sql finished"
bin/gquery watdiv1000.db /home/data/WatDiv/query/L1.sql > L1.txt.bak 2>&1
echo "L1.sql finished"
bin/gquery watdiv1000.db /home/data/WatDiv/query/L2.sql > L2.txt.bak 2>&1
echo "L2.sql finished"
bin/gquery watdiv1000.db /home/data/WatDiv/query/L3.sql > L3.txt.bak 2>&1
echo "L3.sql finished"
bin/gquery watdiv1000.db /home/data/WatDiv/query/S1.sql > S1.txt.bak 2>&1
echo "S1.sql finished"
bin/gquery watdiv1000.db /home/data/WatDiv/query/S2.sql > S2.txt.bak 2>&1
echo "S2.sql finished"
bin/gquery watdiv1000.db /home/data/WatDiv/query/S3.sql > S3.txt.bak 2>&1
echo "S3.sql finished"

echo "big tested"

