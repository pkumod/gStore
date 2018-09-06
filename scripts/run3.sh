bin/gload watdiv4000.db /home/data/WatDiv/watdiv_4000.nt > load.txt 2>&1
echo "load finished"

bin/gquery watdiv4000.db /home/data/WatDiv/query/C1.sql > C1.txt 2>&1
echo "C1.sql finished"
bin/gquery watdiv4000.db /home/data/WatDiv/query/C2.sql > C2.txt 2>&1
echo "C2.sql finished"
bin/gquery watdiv4000.db /home/data/WatDiv/query/C3.sql > C3.txt 2>&1
echo "C3.sql finished"
bin/gquery watdiv4000.db /home/data/WatDiv/query/F1.sql > F1.txt 2>&1
echo "F1.sql finished"
bin/gquery watdiv4000.db /home/data/WatDiv/query/F2.sql > F2.txt 2>&1
echo "F2.sql finished"
bin/gquery watdiv4000.db /home/data/WatDiv/query/F3.sql > F3.txt 2>&1
echo "F3.sql finished"
bin/gquery watdiv4000.db /home/data/WatDiv/query/F4.sql > F4.txt 2>&1
echo "F4.sql finished"
bin/gquery watdiv4000.db /home/data/WatDiv/query/F5.sql > F5.txt 2>&1
echo "F5.sql finished"
bin/gquery watdiv4000.db /home/data/WatDiv/query/L1.sql > L1.txt 2>&1
echo "L1.sql finished"
bin/gquery watdiv4000.db /home/data/WatDiv/query/L2.sql > L2.txt 2>&1
echo "L2.sql finished"
bin/gquery watdiv4000.db /home/data/WatDiv/query/L3.sql > L3.txt 2>&1
echo "L3.sql finished"
bin/gquery watdiv4000.db /home/data/WatDiv/query/L4.sql > L4.txt 2>&1
echo "L4.sql finished"
bin/gquery watdiv4000.db /home/data/WatDiv/query/L5.sql > L5.txt 2>&1
echo "L5.sql finished"
bin/gquery watdiv4000.db /home/data/WatDiv/query/S1.sql > S1.txt 2>&1
echo "S1.sql finished"
bin/gquery watdiv4000.db /home/data/WatDiv/query/S2.sql > S2.txt 2>&1
echo "S2.sql finished"
bin/gquery watdiv4000.db /home/data/WatDiv/query/S3.sql > S3.txt 2>&1
echo "S3.sql finished"
bin/gquery watdiv4000.db /home/data/WatDiv/query/S4.sql > S4.txt 2>&1
echo "S4.sql finished"
bin/gquery watdiv4000.db /home/data/WatDiv/query/S5.sql > S5.txt 2>&1
echo "S5.sql finished"
bin/gquery watdiv4000.db /home/data/WatDiv/query/S6.sql > S6.txt 2>&1
echo "S6.sql finished"
bin/gquery watdiv4000.db /home/data/WatDiv/query/S7.sql > S7.txt 2>&1
echo "S7.sql finished"

echo "big tested"

