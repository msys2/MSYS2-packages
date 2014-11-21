import qbs

CppApplication {
    type: "application" // To suppress bundle generation on Mac
    consoleApplication: true
    files: [
        "main.c",
        "resource.rc",
        "resource.h",
    ]
    cpp.cppFlags: ["-std=c99"]
    Properties {
        condition: qbs.buildVariant == "debug"
        cpp.defines: ["_DEBUG"]
    }

    Properties {
        condition: qbs.buildVariant == "release"
        cpp.defines: ["NDEBUG"]
    }

    cpp.staticLibraries: "psapi"

    Group {     // Properties for the produced executable
        fileTagsFilter: product.type
        qbs.install: true
    }
}
