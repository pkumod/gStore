JavaAPIExample.class: JavaAPIExample.java
	javac -cp ../lib/GstoreJavaAPI.jar JavaAPIExample.java

.PHONY: clean run

run: JavaAPIExample.class
	java -cp ../lib/GstoreJavaAPI.jar:. JavaAPIExample

freebase: Freebase.java
	javac -cp ../lib/GstoreJavaAPI.jar Freebase.java
	java -cp ../lib/GstoreJavaAPI.jar:. Freebase

dbpedia: DBpedia.java
	javac -cp ../lib/GstoreJavaAPI.jar DBpedia.java
	java -cp ../lib/GstoreJavaAPI.jar:. DBpedia

clean:
	rm -f JavaAPIExample.class

