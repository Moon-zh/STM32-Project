@ set source=HelloWorld.java
@ set dest=HelloWorld
@ set classpath=".\\;.\\jar\iot-mqttclient.0.0.1.jar;.\\jar\mqttv3-1.2.0.jar;.\\jar\fastjson-1.2.40.jar;.\\jar\gson-2.7.jar;.\\jar\network-core-0.0.1.jar;.\\jar\iot-linkkit-java.jar;.\\jar\common-utils-0.0.1.jar;.\\jar\device_manager_java.jar;.\\jar\apiclient-0.0.1.jar;.\\jar\okhttp-3.1.0.jar;.\\jar\okio-1.6.0.jar"

echo %classpath%
javac -classpath %classpath% %source%
java  -classpath %classpath% %dest% device_id_password.json


@pause