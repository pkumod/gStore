bin/gload watdiv.db /home/data/WatDiv/database/watdiv_3000.nt > WATDIV/load.txt 2>&1
echo "load finished"

bin/gquery watdiv.db /home/data/WatDiv/query/C1.sql > WATDIV/C1.txt 2>&1
echo "C1.sql finished"
bin/gquery watdiv.db /home/data/WatDiv/query/C2.sql > WATDIV/C2.txt 2>&1
echo "C2.sql finished"
bin/gquery watdiv.db /home/data/WatDiv/query/C3.sql > WATDIV/C3.txt 2>&1
echo "C3.sql finished"

bin/gquery watdiv.db /home/data/WatDiv/query/F1.sql > WATDIV/F1.txt 2>&1
echo "F1.sql finished"
bin/gquery watdiv.db /home/data/WatDiv/query/F2.sql > WATDIV/F2.txt 2>&1
echo "F2.sql finished"
bin/gquery watdiv.db /home/data/WatDiv/query/F3.sql > WATDIV/F3.txt 2>&1
echo "F3.sql finished"

bin/gquery watdiv.db /home/data/WatDiv/query/L1.sql > WATDIV/L1.txt 2>&1
echo "L1.sql finished"
bin/gquery watdiv.db /home/data/WatDiv/query/L2.sql > WATDIV/L2.txt 2>&1
echo "L2.sql finished"
bin/gquery watdiv.db /home/data/WatDiv/query/L3.sql > WATDIV/L3.txt 2>&1
echo "L3.sql finished"

bin/gquery watdiv.db /home/data/WatDiv/query/S1.sql > WATDIV/S1.txt 2>&1
echo "S1.sql finished"
bin/gquery watdiv.db /home/data/WatDiv/query/S2.sql > WATDIV/S2.txt 2>&1
echo "S2.sql finished"
bin/gquery watdiv.db /home/data/WatDiv/query/S3.sql > WATDIV/S3.txt 2>&1
echo "S3.sql finished"

echo "watdiv tested"

