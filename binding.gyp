{
    'targets':[{
        'target_name':'js_enet',
        "cflags!": ["-fno-exceptions"],
        "cflags_cc": ["-fno-exceptions"],
        'defines':["NAPI_CPP_EXCEPTIONS"],      
		'include_dirs':['./enet/include/'],
        'sources':['./js_enet.cpp',
                   './enet/callbacks.c',
                   './enet/compress.c',
                   './enet/host.c',
                   './enet/list.c',
                   './enet/packet.c',
                   './enet/peer.c',
                   './enet/protocol.c',
                   './enet/unix.c',
                   './enet/win32.c']
    }]
} 
