rm Parser/Sparql*
find . -type f -print | grep -E "(makefile|Makefile|\.(c(pp)?|h|sh|py|ini|sql|conf))$" | xargs wc -l

