#include <node.h>
#include <v8.h>

#include <map>
#include <string>

#include "enet/enet.h"

#ifdef _WIN32
#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib, "winmm.lib ")
#endif

static std::map<uint32_t, ENetHost*> ENetHostSet;

void throw_v8_exception(v8::Isolate *isolate, std::string msg) {
    isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, msg.c_str(), v8::NewStringType::kNormal).ToLocalChecked()));
}

void js_enet_initialize(const v8::FunctionCallbackInfo<v8::Value> &args) {
    auto isolate = args.GetIsolate();
    
    if (enet_initialize() != 0){
        throw_v8_exception(isolate, "An error occurred while initializing ENet!");
    }
}

void js_enet_deinitialize(const v8::FunctionCallbackInfo<v8::Value> &args) {
    enet_deinitialize();
}

void js_enet_host_create(const v8::FunctionCallbackInfo<v8::Value> &args) {
    auto isolate = args.GetIsolate();

    if (!args[0]->IsString() || !args[1]->IsUint32() || !args[2]->IsUint32()) {
        throw_v8_exception(isolate, "Wrong arguments");
        return;
    }

    v8::Local<v8::Context> context = v8::Context::New(isolate);
    v8::String::Utf8Value ip(isolate, args[0]);//->ToString(context).ToLocalChecked());
    uint16_t port = args[1]->Uint32Value(context).FromJust();
    uint32_t peercount = args[2]->Uint32Value(context).FromJust();

    ENetAddress address;
    if (enet_address_set_host_ip(&address, *ip) != 0){
        throw_v8_exception(isolate, "enet_address_set_host_ip faild");
        return;
    }
    address.port = port;

    ENetHost* host = enet_host_create(&address, peercount, 1, 0, 0);
    if (host == nullptr){
        throw_v8_exception(isolate, "enet_host_create faild");
        return;
    }

    uint32_t handle = ENetHostSet.size();
    ENetHostSet.insert(std::make_pair(handle, host));

    args.GetReturnValue().Set(handle);
}

void init(v8::Local <v8::Object> exports) {
    NODE_SET_METHOD(exports, "enet_initialize", js_enet_initialize);
    NODE_SET_METHOD(exports, "enet_deinitialize", js_enet_deinitialize);
    NODE_SET_METHOD(exports, "enet_host_create", js_enet_host_create);
}

NODE_MODULE(js_enet, init);
