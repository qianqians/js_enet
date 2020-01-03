#include <node.h>
#include <v8.h>
#include <string>

#include "enet/enet.h"

#ifdef _WIN32
#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib, "winmm.lib ")
#endif

void throw_v8_exception(v8::Isolate *isolate, std::string msg) {
    isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, msg.c_str())));
}

void js_enet_initialize(const v8::FunctionCallbackInfo <v8::Value> &args) {
    auto isolate = args.GetIsolate();
    
    if (enet_initialize() != 0){
        throw_v8_exception(isolate, "An error occurred while initializing ENet!");
    }
}

void init(v8::Local <v8::Object> exports) {
    NODE_SET_METHOD(exports, "enet_initialize", js_enet_initialize);
}

NODE_MODULE(js_enet, init);
