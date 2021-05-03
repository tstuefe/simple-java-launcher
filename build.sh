gcc -g -O0 -I /shared/projects/openjdk/jdks/sapmachine16/include -I /shared/projects/openjdk/jdks/sapmachine16/include/linux launcher.c -ldl -o launcher

# run : launcher <jvm path> [<optional vm opts>]
# eg  ./launcher /shared/projects/openjdk/jdk-jdk/output-slowdebug/images/jdk/lib/server/libjvm.so

