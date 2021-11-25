all: POSTexample.class GETexample.class Benchmark.class

Benchmark.class: Benchmark.java
	javac -encoding utf8 -cp ../lib/GstoreJavaAPI.jar Benchmark.java

POSTexample.class: POSTexample.java
	javac -encoding utf8 -cp ../lib/GstoreJavaAPI.jar POSTexample.java

GETexample.class: GETexample.java
	javac -encoding utf8 -cp ../lib/GstoreJavaAPI.jar GETexample.java

.PHONY: clean POST-example GET-example Benchmark

POST-example: POSTexample.class
	java -cp ../lib/GstoreJavaAPI.jar:. POSTexample

GET-example: GETexample.class
	java -cp ../lib/GstoreJavaAPI.jar:. GETexample

Benchmark: Benchmark.class
	java -XX:-UseGCOverheadLimit -Xmx64G -cp ../lib/GstoreJavaAPI.jar:. Benchmark

clean:
	rm -rf *.class

