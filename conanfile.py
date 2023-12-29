from conan import ConanFile
# from conan.tools.cmake.layout import cmake_layout

class GStore(ConanFile):
    ### Project Attributes ###
    # references: https://docs.conan.io/2/reference/conanfile/attributes.html
    name = "gstore"
    version = "1.2"
    license = "BSD-3-Clause"
    homepage = "https://gstore.cn"
    url = "https://github.com/pkumod/gStore"

    ### Dependencies ###
    # available packages: https://conconan.io/center/recipes
    # usage reference: https://github.com/conan-io/conan-center-index/blob/master/recipes/{PACKAGE_NAME}/all
    requires = (
        "log4cplus/2.1.0",
        "zlib/1.3",
        "boost/1.83.0",
        "indicators/2.3",
        "rapidjson/1.1.0",
        "libcurl/8.5.0",
        "openssl/3.2.0",
        "minizip/1.2.13",
    )
    build_policy = "missing"
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps", "CMakeToolchain"

    def configure(self):
        # set the project to use C++17
        self.settings.compiler.cppstd = "17"
        # set log4cplus to use char instead of wchar_t
        self.options["log4cplus"].unicode = False

    def package_info(self):
        self.output.info("PackageInfo!: Cppstd version: %s!" % self.settings.compiler.cppstd)

    # def layout(self):
    #     # set the project layout
    #     cmake_layout(self)