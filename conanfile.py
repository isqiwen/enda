from conan import ConanFile
from conan.errors import ConanInvalidConfiguration
from conan.tools.cmake import CMake, CMakeToolchain, CMakeDeps, cmake_layout
from conan.tools.build import check_min_cppstd

class Enda(ConanFile):

    name = "Enda"
    version = "1.0.0"
    label = "Efficient NdArray"

    # Optional metadata
    license = "<Put the package license here>"
    author = "<Put your name here> <And your email here>"
    url = "<Package recipe repository url here, for issues about the package>"
    description = "<Description of hello package here>"
    topics = ("<Put some tag here>", "<here>", "<and here>")

    settings = "os", "compiler", "build_type", "arch"

    exports_sources = "CMakeLists.txt", "cmake/*", "src/*", "test/*"

    options = {"shared": [True, False], "fPIC": [True, False]}
    default_options = {
        "shared": True,
        "fPIC": True
    }

    # requires = [
    #     "zlib/1.3.1",
    #     "boost/1.76.0"
    # ]

    # tool_requires = [
    #     "cmake/[>=3.27.0]",
    #     "ccache/[>=4.8.3]",
    #     "cppcheck/[>=2.12.1]"
    # ]

    test_requires = [
        "gtest/[>=1.14.0]"
    ]

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def configure(self):
        self.options["boost"].shared = False
        self.options["boost"].without_thread = False

    def layout(self):
        cmake_layout(self)

    def validate(self):
        check_min_cppstd(self, "20")

        if self.settings.compiler == "gcc" and self.settings.compiler.version < "9":
            raise ConanInvalidConfiguration("GCC < 9 is not supported.")

    def generate(self):
        tc = CMakeToolchain(self)
        tc.variables["BUILD_SHARED_LIBS"] = self.options.shared
        tc.generate()
        CMakeDeps(self).generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()
