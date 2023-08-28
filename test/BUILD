cc_test(
    name = "TestEventLoop",
    srcs = select({
        "@platforms//os:windows": [
            "TestWSAEvent.cpp",
        ],
        "@platforms//os:linux": [
            "TestEpollEvent.cpp",
        ],
        "@platforms//os:osx": [
            "TestKqueueEvent.cpp",
        ],
    }),
    copts = select({
        "@platforms//os:windows": [
            "/utf-8",
        ],
        "//conditions:default": [
            "-std=c++17"
        ],
    }),
    linkopts = select({
        "@platforms//os:windows": [
            "-DEFAULTLIB:ws2_32.lib",
        ],
        "//conditions:default": [],
    }),
    deps = [
        "@com_google_googletest//:gtest_main",
        "@com_sese_event//:event",
    ],
)
