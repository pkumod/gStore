bin/gload watdiv5000.db ~/watdiv_5000.nt > watdiv5000/load.txt 2>&1
echo "load finished"

bin/gquery watdiv5000.db /home/data/WatDiv/query/C1.sql > watdiv5000/C1.txt 2>&1
echo "C1.sql finished"
bin/gquery watdiv5000.db /home/data/WatDiv/query/C2.sql > watdiv5000/C2.txt 2>&1
echo "C2.sql finished"
bin/gquery watdiv5000.db /home/data/WatDiv/query/C3.sql > watdiv5000/C3.txt 2>&1
echo "C3.sql finished"
bin/gquery watdiv5000.db /home/data/WatDiv/query/F1.sql > watdiv5000/F1.txt 2>&1
echo "F1.sql finished"
bin/gquery watdiv5000.db /home/data/WatDiv/query/F2.sql > watdiv5000/F2.txt 2>&1
echo "F2.sql finished"
bin/gquery watdiv5000.db /home/data/WatDiv/query/F3.sql > watdiv5000/F3.txt 2>&1
echo "F3.sql finished"
bin/gquery watdiv5000.db /home/data/WatDiv/query/F4.sql > watdiv5000/F4.txt 2>&1
echo "F4.sql finished"
bin/gquery watdiv5000.db /home/data/WatDiv/query/F5.sql > watdiv5000/F5.txt 2>&1
echo "F5.sql finished"
bin/gquery watdiv5000.db /home/data/WatDiv/query/L1.sql > watdiv5000/L1.txt 2>&1
echo "L1.sql finished"
bin/gquery watdiv5000.db /home/data/WatDiv/query/L2.sql > watdiv5000/L2.txt 2>&1
echo "L2.sql finished"
bin/gquery watdiv5000.db /home/data/WatDiv/query/L3.sql > watdiv5000/L3.txt 2>&1
echo "L3.sql finished"
bin/gquery watdiv5000.db /home/data/WatDiv/query/L4.sql > watdiv5000/L4.txt 2>&1
echo "L4.sql finished"
bin/gquery watdiv5000.db /home/data/WatDiv/query/L5.sql > watdiv5000/L5.txt 2>&1
echo "L5.sql finished"
bin/gquery watdiv5000.db /home/data/WatDiv/query/S1.sql > watdiv5000/S1.txt 2>&1
echo "S1.sql finished"
bin/gquery watdiv5000.db /home/data/WatDiv/query/S2.sql > watdiv5000/S2.txt 2>&1
echo "S2.sql finished"
bin/gquery watdiv5000.db /home/data/WatDiv/query/S3.sql > watdiv5000/S3.txt 2>&1
echo "S3.sql finished"
bin/gquery watdiv5000.db /home/data/WatDiv/query/S4.sql > watdiv5000/S4.txt 2>&1
echo "S4.sql finished"
bin/gquery watdiv5000.db /home/data/WatDiv/query/S5.sql > watdiv5000/S5.txt 2>&1
echo "S5.sql finished"
bin/gquery watdiv5000.db /home/data/WatDiv/query/S6.sql > watdiv5000/S6.txt 2>&1
echo "S6.sql finished"
bin/gquery watdiv5000.db /home/data/WatDiv/query/S7.sql > watdiv5000/S7.txt 2>&1
echo "S7.sql finished"

echo "big tested"

