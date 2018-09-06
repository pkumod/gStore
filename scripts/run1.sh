bin/gload lubm5000.db /home/data/LUBM/database/lubm_5000.nt > load.txt 2>&1
echo "load finished"

bin/gsub lubm5000.db ../LUBM5000_small > sub.txt 2>&1
echo "subed"

bin/gquery lubm5000.db /home/data/LUBM/query/q0.sql > q0.txt 2>&1
echo "q0.sql finished"
bin/gquery lubm5000.db /home/data/LUBM/query/q1.sql > q1.txt 2>&1
echo "q1.sql finished"
bin/gquery lubm5000.db /home/data/LUBM/query/q2.sql > q2.txt 2>&1
echo "q2.sql finished"
bin/gquery lubm5000.db /home/data/LUBM/query/q3.sql > q3.txt 2>&1
echo "q3.sql finished"
bin/gquery lubm5000.db /home/data/LUBM/query/q4.sql > q4.txt 2>&1
echo "q4.sql finished"
bin/gquery lubm5000.db /home/data/LUBM/query/q5.sql > q5.txt 2>&1
echo "q5.sql finished"

bin/gadd lubm5000.db ../LUBM5000_small > add.txt 2>&1
echo "added"

bin/gquery lubm5000.db /home/data/LUBM/query/q0.sql > q0.txt.bak 2>&1
echo "q0.sql finished"
bin/gquery lubm5000.db /home/data/LUBM/query/q1.sql > q1.txt.bak 2>&1
echo "q1.sql finished"
bin/gquery lubm5000.db /home/data/LUBM/query/q2.sql > q2.txt.bak 2>&1
echo "q2.sql finished"
bin/gquery lubm5000.db /home/data/LUBM/query/q3.sql > q3.txt.bak 2>&1
echo "q3.sql finished"
bin/gquery lubm5000.db /home/data/LUBM/query/q4.sql > q4.txt.bak 2>&1
echo "q4.sql finished"
bin/gquery lubm5000.db /home/data/LUBM/query/q5.sql > q5.txt.bak 2>&1
echo "q5.sql finished"

echo "big tested"

