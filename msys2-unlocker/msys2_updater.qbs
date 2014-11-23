import qbs

CppApplication {
    type: "application" // To suppress bundle generation on Mac
    consoleApplication: true
    files: [
        "main.c",
        "splash/convert-all-xcf-to-jpg",
        "resource.rc",
        "resource.h",
        "splash_frames.h",
        "splash_frames.rc2",
    ]
    cpp.cppFlags: ["-std=c99"]
    Properties {
        condition: qbs.buildVariant == "debug"
        cpp.defines: ["_DEBUG", "__USE_MINGW_ANSI_STDIO=1"]
    }

    Properties {
        condition: qbs.buildVariant == "release"
        cpp.defines: ["NDEBUG", "__USE_MINGW_ANSI_STDIO=1"]
    }

    cpp.staticLibraries: ["psapi", "gdiplus", "ole32"]

    Group {     // Properties for the produced executable
        fileTagsFilter: product.type
        qbs.install: true
    }
}
