all: JavaAPIExample.class Benchmark.class

JavaAPIExample.class: JavaAPIExample.java
	javac -cp ../lib/GstoreJavaAPI.jar JavaAPIExample.java

Benchmark.class: Benchmark.java
	javac -cp ../lib/GstoreJavaAPI.jar Benchmark.java

.PHONY: clean run

run: JavaAPIExample.class
	java -cp ../lib/GstoreJavaAPI.jar:. JavaAPIExample
test: Benchmark.class
	java -XX:-UseGCOverheadLimit -Xmx64G -cp ../lib/GstoreJavaAPI.jar:. Benchmark

freebase: Freebase.java
	javac -cp ../lib/GstoreJavaAPI.jar Freebase.java
	java -cp ../lib/GstoreJavaAPI.jar:. Freebase

dbpedia: DBpedia.java
	javac -cp ../lib/GstoreJavaAPI.jar DBpedia.java
	java -cp ../lib/GstoreJavaAPI.jar:. DBpedia

clean:
	rm -f JavaAPIExample.class

