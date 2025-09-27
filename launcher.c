#include "jni.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "dlfcn.h"

#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>

#define trc(...)        { printf(__VA_ARGS__); printf("\n"); fflush(stdout); }
#define errbye(...)     { trc(__VA_ARGS__); exit(0); }

static void* libhdl = NULL;
static void* cjvmhdl = NULL;

static JavaVM* jvm = NULL;
static JNIEnv* env = NULL;

typedef jint (*JNI_CreateJavaVMFkt)(JavaVM **p_vm, void **p_env, void *vm_args);

static jint dyn_JNI_CreateJavaVM(JavaVM** p_vm, void** p_env, void* vm_args) {
    return ((JNI_CreateJavaVMFkt)cjvmhdl)(p_vm, p_env, vm_args);
}

static void* other_thread(void* dummy) {
	char* name = dummy;
	trc("I am also alive name=%s %lu", name, (unsigned long)pthread_self());
	void* p_env = NULL;
        JavaVMAttachArgs args;
	args.version = JNI_VERSION_1_2;
	args.name = name;
	args.group = NULL;
	jint rc = (*jvm)->AttachCurrentThread(jvm, &p_env, strlen(name) == 0 ? NULL : &args);
	if (rc == JNI_OK) {
		trc("I am also attached name=%s %lu", name, (unsigned long)pthread_self());
	} else {
		trc("I failed to attach name=%s (%d) %lu", name, rc, (unsigned long)pthread_self());
	}
	
	for (;;) {
		sleep(1);
	}
}

static void start_other_thread(const char* name) {
	int rc = 0;
	pthread_t tid;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	rc = pthread_create(&tid, &attr, other_thread, (void*)name);
	if (rc == 0) {
		trc("started other thread %lu", (unsigned long)pthread_self());
	} else {
		trc("thread start failure %d", errno);
	}
}


static void* resolve_function(const char* name) {
    void* hdl = NULL;
    trc("Resolving %s...", name)
    hdl = dlsym(libhdl, name);
    if (hdl == NULL) {
        errbye("Failed to find \"%s\" (%s)", name, dlerror());
    } else {
        trc("Resolved %s: %p", name, hdl);
    }
    return hdl;
}

static void blockall() {
{ sigset_t s;
const int sigs[] = { SIGXFSZ, SIGSEGV, SIGBUS, SIGFPE, SIGPIPE, SIGILL, SIGQUIT, SIGTERM, SIGINT, SIGHUP, SIGUSR2, SIGABRT , -1};
for (int i = 0; sigs[i] != -1; i++) {
sigaddset(&s, sigs[i]);
}
pthread_sigmask(SIG_BLOCK, &s, NULL);
}
}

int main(int argc, char** argv) {
    
    JavaVMInitArgs vm_args;
    int num_vm_options;
    JavaVMOption* options;
    int i;
    jint res;

	blockall();

    if (argc <= 1) {
        errbye("Usage: %s <jvm path> [vm options]", argv[0]);
    }

    trc("I am PID %d", getpid());

    trc("Loading %s..", argv[1]);
    libhdl = dlopen(argv[1], RTLD_NOW);
    if (libhdl == NULL) {
        errbye("Failed to load \"%s\" (%s)", argv[1], dlerror());
    }

    cjvmhdl = resolve_function("JNI_CreateJavaVM");
    
    // Prepare options
    num_vm_options = argc - 2;
    options = calloc(num_vm_options, sizeof(JavaVMOption));
    for (i = 0; i < num_vm_options; i ++) {
        options[i].optionString = strdup(argv[i + 2]);
        trc("Option: %s", options[i].optionString);
    }

    // Prepare args struct
    vm_args.version = JNI_VERSION_1_2;
    vm_args.options = options;
    vm_args.nOptions = num_vm_options;
    vm_args.ignoreUnrecognized = 0;

    // Call CJVM
    trc("Calling CreateJavaVM...");
    res = dyn_JNI_CreateJavaVM(&jvm, (void**)&env, &vm_args);
    if (res == JNI_OK) {
        trc("Success");
    } else {
        errbye("Error (%d)", res);
    }

    // Wait for key press    
    trc("press any key...");
    getc(stdin);

    // Start some more threads
    trc("Starting more threads and attaching them...");
    start_other_thread("");
    start_other_thread("thread1");
    start_other_thread("thread lengthy name 222222222222222222");

    // Wait for key press    
    trc("press any key...");
    getc(stdin);

    // destroy VM
    res = ((*jvm)->DestroyJavaVM)(jvm);
    if (res == JNI_OK) {
        trc("Success");
    } else {
        errbye("Error (%d)", res);
    }

    // Wait for key press    
    trc("press any key...");
    getc(stdin);

    return 0;

}

