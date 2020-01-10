#!/bin/sh

path=$0
last_char=${path:${#path}-1}

path=${path%/*}
classpath=$path
classpath=$classpath":"$path"/jar/iot-mqttclient.0.0.1.jar"
classpath=$classpath":"$path"/jar/mqttv3-1.2.0.jar"
classpath=$classpath":"$path"/jar/fastjson-1.2.40.jar"
classpath=$classpath":"$path"/jar/gson-2.7.jar"
classpath=$classpath":"$path"/jar/network-core-0.0.1.jar"
classpath=$classpath":"$path"/jar/iot-linkkit-java.jar"
classpath=$classpath":"$path"/jar/common-utils-0.0.1.jar"
classpath=$classpath":"$path"/jar/device_manager_java.jar"
classpath=$classpath":"$path"/jar/okhttp-3.1.0.jar"
classpath=$classpath":"$path"/jar/apiclient-0.0.1.jar"
classpath=$classpath":"$path"/jar/okio-1.6.0.jar"

source=$path"/HelloWorld.java"
dest="HelloWorld"

echo javac -classpath \"$classpath\" $source
javac -classpath "$classpath" $source
java  -classpath "$classpath" $dest $path"/device_id_password.json"

