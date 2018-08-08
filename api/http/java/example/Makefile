all: JavaAPIExample.class Benchmark.class Benchmark1.class

JavaAPIExample.class: JavaAPIExample.java
	javac -cp ../lib/GstoreJavaAPI.jar JavaAPIExample.java

Benchmark.class: Benchmark.java
	javac -cp ../lib/GstoreJavaAPI.jar Benchmark.java

Benchmark1.class: Benchmark1.java
	javac -cp ../lib/GstoreJavaAPI.jar Benchmark1.java

.PHONY: clean run

run: JavaAPIExample.class
	java -cp ../lib/GstoreJavaAPI.jar:. JavaAPIExample
test: Benchmark.class
	java -XX:-UseGCOverheadLimit -Xmx64G -cp ../lib/GstoreJavaAPI.jar:. Benchmark
test1: Benchmark1.class
	java -XX:-UseGCOverheadLimit -Xmx64G -cp ../lib/GstoreJavaAPI.jar:. Benchmark1

#test: Benchmark.class
	#java -XX:-UseGCOverheadLimit -Xmx64G -cp ../lib/GstoreJavaAPI.jar:. Benchmark >> time.txt
clean:
	rm -f JavaAPIExample.class

