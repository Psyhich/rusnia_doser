from conans import ConanFile, tools, CMake
from conan.tools.cmake import CMakeToolchain
from conans.errors import ConanInvalidConfiguration
import os


class RusniaDoserConan(ConanFile):
    name = "rusnia_doser"
    version = "0.1"
    homepage = "https://github.com/Psyhich/rusnia_doser"

    requires = "libcurl/7.84.0"
    generators = "cmake"
    default_options = {"*:shared": False}

    # Binary configuration
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "fPIC": [True, False]}
    default_options = {"shared": False, "fPIC": True}

    exports_sources = "CMakeLists.txt", "main.cpp", "config/*",\
                      "src/*", "include/*", "conanfile.txt"

    def validate(self):
        if self.settings.os != "Linux":
            raise ConanInvalidConfiguration("Currently it's only buildable on Linux")

    def source(self):
        os.mkdir("lib")
        with tools.chdir("lib"):
            os.mkdir("args-parser")
            with tools.chdir("args-parser"):
                tools.Git().clone("https://github.com/igormironchik/args-parser.git",
                                  branch="2ecbf646b0a823dffe83959cc5b21ed89e48f14f",
                                  shallow=True)

            os.mkdir("json")
            with tools.chdir("json"):
                tools.Git().clone("https://github.com/nlohmann/json.git",
                                  branch="e4643d1f1b03fc7a1d7b65f17e012ca93680cad8",
                                  shallow=True)

            os.mkdir("spdlog")
            with tools.chdir("spdlog"):
                tools.Git().clone("https://github.com/gabime/spdlog.git",
                                  branch="d7690d8e7eed721d78b52e032e996a5d1ef47d6f",
                                  shallow=True)

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
