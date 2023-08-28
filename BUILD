cc_library(
    name = "event",
    srcs = select({
        "@platforms//os:windows": [
            "src/WSAEventConvert.cpp",
            "src/WSAEventLoop.cpp",
        ],
        "@platforms//os:linux": [
            "src/EpollEventConvert.cpp",
            "src/EpollEventLoop.cpp",
        ],
        "@platforms//os:osx": [
            "src/KqueueEventLoop.cpp",
        ],
    }),
    hdrs = select({
        "@platforms//os:windows": [
            "include/sese/event/WSAEvent.h",
            "include/sese/event/WSAEventConvert.h",
            "include/sese/event/WSAEventLoop.h",
        ],
        "@platforms//os:linux": [
            "include/sese/event/EpollEvent.h",
            "include/sese/event/EpollEventConvert.h",
            "include/sese/event/EpollEventLoop.h",
        ],
        "@platforms//os:osx": [
            "include/sese/event/KqueueEvent.h",
            "include/sese/event/KqueueEventLoop.h",
        ],
    }) + [
    	"include/sese/event/Event.h",
    	"include/sese/event/BaseEvent.h",
    	"include/sese/event/BaseEventConvert.h",
    	"include/sese/event/BaseEventLoop.h",
    ],
    copts = select({
        "@platforms//os:windows": [
            "/utf-8",
        ],
	"//conditions:default": [
	    "-std=c++11"
	],
    }),
    includes = [
        "include",
    ],
    visibility = ["//visibility:public"],
)
