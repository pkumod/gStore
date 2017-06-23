lib_dir=../lib/
socket_obj_dir=../../../.objs/

all: $(lib_dir)GstoreJavaAPI.jar

$(lib_dir)GstoreJavaAPI.jar: jgsc/GstoreConnector.class
	jar -cvf $(lib_dir)GstoreJavaAPI.jar jgsc/GstoreConnector.java jgsc/GstoreConnector.class

jgsc/GstoreConnector.class:
	javac jgsc/GstoreConnector.java 

clean:
	rm -rf $(lib_dir)GstoreJavaAPI.jar jgsc/GstoreConnector.class

