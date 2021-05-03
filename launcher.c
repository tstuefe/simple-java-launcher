#include "jni.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "dlfcn.h"

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

int main(int argc, char** argv) {
    
    JavaVMInitArgs vm_args;
    int num_vm_options;
    JavaVMOption* options;
    int i;
    jint res;

    if (argc <= 1) {
        errbye("Usage: %s <jvm path> [vm options]", argv[0]);
    }

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

