lib_dir=../lib/
socket_obj_dir=../../../.objs/

GstoreJavaAPI.jar: JGSC
	jar -cvf $(lib_dir)GstoreJavaAPI.jar jgsc/GstoreConnector.java jgsc/GstoreConnector.class

JGSC:
	javac jgsc/GstoreConnector.java 

clean:
	rm -rf $(lib_dir)GstoreJavaAPI.jar jgsc/GstoreConnector.class

