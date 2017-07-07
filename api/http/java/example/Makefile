JavaAPIExample.class:
	javac -cp ../lib/GstoreJavaAPI.jar JavaAPIExample.java

.PHONY: clean run

run: JavaAPIExample.class
	java -cp ../lib/GstoreJavaAPI.jar:. JavaAPIExample

clean:
	rm -f JavaAPIExample.class

