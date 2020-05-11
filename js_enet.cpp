#include <node.h>
#include <node_buffer.h>
#include <v8.h>

#include <map>
#include <string>
#include <iostream>

#include "enet/enet.h"

#ifdef _WIN32
#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib, "winmm.lib ")
#endif

static uint32_t handle = 0;
static std::map<uint32_t, ENetHost*> ENetHostSet;
static std::map<uint64_t, ENetPeer*> ENetPeerSet;

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
    v8::String::Utf8Value ip(isolate, args[0]);
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

    auto _h = handle++;
    ENetHostSet.insert(std::make_pair(_h, host));

    args.GetReturnValue().Set(_h);
}

void js_enet_client_create(const v8::FunctionCallbackInfo<v8::Value> &args) {
    auto isolate = args.GetIsolate();

    if (!args[0]->IsUint32()) {
        throw_v8_exception(isolate, "Wrong arguments");
        return;
    }

    v8::Local<v8::Context> context = v8::Context::New(isolate);
    uint32_t peercount = args[0]->Uint32Value(context).FromJust();

    ENetHost* host = enet_host_create(nullptr, peercount, 1, 0, 0);
    if (host == nullptr){
        throw_v8_exception(isolate, "enet_host_create faild");
        return;
    }

    auto _h = handle++;
    ENetHostSet.insert(std::make_pair(_h, host));

    args.GetReturnValue().Set(_h);
}

void js_enet_host_service(const v8::FunctionCallbackInfo<v8::Value> &args) {
    auto isolate = args.GetIsolate();

    if (!args[0]->IsUint32()) {
        throw_v8_exception(isolate, "Wrong arguments");
        return;
    }

    v8::Local<v8::Context> context = v8::Context::New(isolate);
    uint32_t _h = args[0]->Uint32Value(context).FromJust();
    ENetHost* host = ENetHostSet[_h];

    ENetEvent event;
    if (enet_host_service(host, &event, 10) > 0)
    {
        v8::Local<v8::Object> obj = v8::Object::New(isolate);

        v8::Local<v8::Integer> type = v8::Uint32::NewFromUnsigned(isolate, (uint32_t)event.type);
        obj->Set(context, v8::String::NewFromUtf8(isolate, "type", v8::NewStringType::kNormal).ToLocalChecked(), type);

        switch (event.type)
        {
        case ENET_EVENT_TYPE_CONNECT:
            {
                char ip[256];
                enet_address_get_host_ip(&event.peer->address, ip, 256);
                v8::Local<v8::String> _ip = v8::String::NewFromUtf8(isolate, ip, v8::NewStringType::kNormal).ToLocalChecked();
                v8::Local<v8::Integer> host = v8::Uint32::NewFromUnsigned(isolate, (uint32_t)event.peer->address.host);
                v8::Local<v8::Integer> port = v8::Uint32::NewFromUnsigned(isolate, (uint32_t)event.peer->address.port);
                obj->Set(context, v8::String::NewFromUtf8(isolate, "host", v8::NewStringType::kNormal).ToLocalChecked(), host);
                obj->Set(context, v8::String::NewFromUtf8(isolate, "ip", v8::NewStringType::kNormal).ToLocalChecked(), _ip);
                obj->Set(context, v8::String::NewFromUtf8(isolate, "port", v8::NewStringType::kNormal).ToLocalChecked(), port);

                uint64_t peerHandle = (uint64_t)event.peer->address.host << 32 | event.peer->address.port;
                ENetPeerSet.insert(std::make_pair(peerHandle, event.peer));
            }
            break;
        case ENET_EVENT_TYPE_RECEIVE:
            {
                char ip[256];
                enet_address_get_host_ip(&event.peer->address, ip, 256);
                v8::Local<v8::String> _ip = v8::String::NewFromUtf8(isolate, ip, v8::NewStringType::kNormal).ToLocalChecked();
                v8::Local<v8::Integer> host = v8::Uint32::NewFromUnsigned(isolate, (uint32_t)event.peer->address.host);
                v8::Local<v8::Integer> port = v8::Uint32::NewFromUnsigned(isolate, (uint32_t)event.peer->address.port);
                obj->Set(context, v8::String::NewFromUtf8(isolate, "host", v8::NewStringType::kNormal).ToLocalChecked(), host);
                obj->Set(context, v8::String::NewFromUtf8(isolate, "ip", v8::NewStringType::kNormal).ToLocalChecked(), _ip);
                obj->Set(context, v8::String::NewFromUtf8(isolate, "port", v8::NewStringType::kNormal).ToLocalChecked(), port);

                v8::MaybeLocal<v8::Object> data = node::Buffer::Copy(isolate, (char*)event.packet -> data, event.packet -> dataLength);
                obj->Set(context, v8::String::NewFromUtf8(isolate, "data", v8::NewStringType::kNormal).ToLocalChecked(), data.ToLocalChecked());

                enet_packet_destroy (event.packet);
            }
            break;
        
        case ENET_EVENT_TYPE_DISCONNECT:
            {
                char ip[256];
                enet_address_get_host_ip(&event.peer->address, ip, 256);
                v8::Local<v8::String> _ip = v8::String::NewFromUtf8(isolate, ip, v8::NewStringType::kNormal).ToLocalChecked();
                v8::Local<v8::Integer> host = v8::Uint32::NewFromUnsigned(isolate, (uint32_t)event.peer->address.host);
                v8::Local<v8::Integer> port = v8::Uint32::NewFromUnsigned(isolate, (uint32_t)event.peer->address.port);
                obj->Set(context, v8::String::NewFromUtf8(isolate, "host", v8::NewStringType::kNormal).ToLocalChecked(), host);
                obj->Set(context, v8::String::NewFromUtf8(isolate, "ip", v8::NewStringType::kNormal).ToLocalChecked(), _ip);
                obj->Set(context, v8::String::NewFromUtf8(isolate, "port", v8::NewStringType::kNormal).ToLocalChecked(), port);

                uint64_t peerHandle = (uint64_t)event.peer->address.host << 32 | event.peer->address.port;
                ENetPeerSet.erase(peerHandle);
            }
            break;
        }
        args.GetReturnValue().Set(obj);
        return;
    }

    args.GetReturnValue().SetNull();
}

void js_enet_peer_send(const v8::FunctionCallbackInfo<v8::Value> &args) {
    auto isolate = args.GetIsolate();

    if (!args[0]->IsUint32() || !args[1]->IsUint32() || !args[2]->IsUint32()) {
        throw_v8_exception(isolate, "Wrong arguments");
        return;
    }
    
    v8::Local<v8::Context> context = v8::Context::New(isolate);
    uint32_t _h = args[0]->Uint32Value(context).FromJust();
    uint32_t _host = args[1]->Uint32Value(context).FromJust();
    uint16_t _port = args[2]->Uint32Value(context).FromJust();

    ENetHost* host = ENetHostSet[_h];

    uint64_t peerHandle = (uint64_t)_host << 32 | _port;
    ENetPeer* peer = ENetPeerSet[peerHandle];

    char* data = node::Buffer::Data(args[3]);
    size_t len = node::Buffer::Length(args[3]);

    ENetPacket* packet = enet_packet_create(data, len, ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(peer, 0, packet);
    enet_host_flush(host);
}

void js_enet_host_connect(const v8::FunctionCallbackInfo<v8::Value> &args){
    auto isolate = args.GetIsolate();

    if (!args[0]->IsUint32() || !args[1]->IsString() || !args[2]->IsUint32()) {
        throw_v8_exception(isolate, "Wrong arguments");
        return;
    }

    v8::Local<v8::Context> context = v8::Context::New(isolate);
    uint32_t _h = args[0]->Uint32Value(context).FromJust();
    v8::String::Utf8Value _ip(isolate, args[1]);
    uint16_t _port = args[2]->Uint32Value(context).FromJust();

    ENetHost* host = ENetHostSet[_h];

    ENetAddress address;
    if (enet_address_set_host_ip(&address, *_ip) != 0){
        throw_v8_exception(isolate, "enet_address_set_host_ip faild");
        return;
    }
    address.port = _port;

    ENetPeer* peer = enet_host_connect(host, &address, 1, 0);
    if (peer == nullptr){
        throw_v8_exception(isolate, "enet_host_connect faild");
    }
}


void init(v8::Local <v8::Object> exports) {
    NODE_SET_METHOD(exports, "enet_initialize", js_enet_initialize);
    NODE_SET_METHOD(exports, "enet_deinitialize", js_enet_deinitialize);
    NODE_SET_METHOD(exports, "enet_host_create", js_enet_host_create);
    NODE_SET_METHOD(exports, "enet_client_create", js_enet_client_create);
    NODE_SET_METHOD(exports, "enet_host_service", js_enet_host_service);
    NODE_SET_METHOD(exports, "enet_peer_send", js_enet_peer_send);
    NODE_SET_METHOD(exports, "enet_host_connect", js_enet_host_connect);
}

NODE_MODULE(js_enet, init);
