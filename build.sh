gcc -g -O0 -I /shared/projects/openjdk/jdks/sapmachine24/include -I /shared/projects/openjdk/jdks/sapmachine24/include/linux launcher.c -ldl -lpthread -o launcher

# run : launcher <jvm path> [<optional vm opts>]
# eg  ./launcher /shared/projects/openjdk/jdk-jdk/output-slowdebug/images/jdk/lib/server/libjvm.so

