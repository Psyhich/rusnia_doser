from conans import ConanFile, CMake
from conan.tools.cmake import CMakeToolchain
from conans.errors import ConanInvalidConfiguration


class RusniaDoserConan(ConanFile):
    name = "rusnia_doser"
    version = "0.1"
    homepage = "https://github.com/Psyhich/rusnia_doser"

    requires = "libcurl/7.84.0", "spdlog/1.11.0",\
        "args-parser/6.3.2"
    generators = "cmake_find_package"
    default_options = {"*:shared": False}

    # Binary configuration
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "fPIC": [True, False]}
    default_options = {"shared": False, "fPIC": True}

    exports_sources = "CMakeLists.txt", "main.cpp", "config/*",\
                      "src/*", "conanfile.txt"

    def validate(self):
        if self.settings.os != "Linux":
            raise ConanInvalidConfiguration("Currently it's only buildable on Linux")

    def generate(self):
        tc = CMakeToolchain(self)
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()
