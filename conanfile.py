from conan import ConanFile

class GStore(ConanFile):
    ### Project Attributes ###
    # references: https://docs.conan.io/2/reference/conanfile/attributes.html
    name = "gstore"
    version = "2.0"
    license = "BSD-3-Clause"
    homepage = "https://gstore.cn"
    url = "https://github.com/pkumod/gStore"

    ### Dependencies ###
    # available packages: https://conconan.io/center/recipes
    # usage reference: https://github.com/conan-io/conan-center-index/blob/master/recipes/{PACKAGE_NAME}/all
    requires = (
        "rapidjson/1.1.0",
        # antlr4-cppruntime version > 4.9.3, please use CXX17 
        "antlr4-cppruntime/4.9.3",
        "log4cplus/2.0.8",
        "indicators/2.0",
        "zlib/1.3",
        "minizip/1.3.1",
        "jemalloc/5.2.1",
        "readline/8.2",
    )
    build_policy = "missing"
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps", "CMakeToolchain"

    def configure(self):
        # set the project to use C++ standard
        self.settings.compiler.cppstd = "11"
        # set log4cplus to use char instead of wchar_t
        self.options["log4cplus"].unicode = False
        # remove boost from requirements if on loongarch64

    def requirements(self):
        if self.settings.get_safe("arch") != "loongarch64":
            self.requires("boost/1.74.0")
        else:
            self.output.warning("The official Boost wheel (1.83.0) is yet to be fixed on loongarch. "
                             "Please install boost from your system package manager.")

    def package_info(self):
        self.output.info("PackageInfo!: Cppstd version: %s!" % self.settings.compiler.cppstd)

    def layout(self):
        # basically a trimmed down version of the basic_layout function
        _build_type = self.settings.get_safe("build_type")
        if _build_type:
            self.folders.build = "cmake-build-{}".format(str(_build_type).lower())
        else:
            self.folders.build = "cmake-build"
        self.folders.source = "."
        self.folders.generators = self.folders.build
        self.cpp.build.bindirs = ["bin", "bin_tests"]
        self.cpp.build.libdirs = ["lib"]
