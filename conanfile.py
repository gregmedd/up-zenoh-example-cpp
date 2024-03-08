from conan import ConanFile

class HelloWorldRecipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps", "CMakeToolchain", "PkgConfigDeps", "VirtualRunEnv", "VirtualBuildEnv"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "build_testing": [True, False],
        "build_unbundled": [True, False],
        "build_cross_compiling": [True, False],
    }

    default_options = {
        "shared": False,
        "fPIC": False,
        "build_testing": False,
        "build_unbundled": True,
        "build_cross_compiling": False,
    }

    # def configure(self):
    #     self.options["spdlog"].shared = self.options.shared

    def requirements(self):
        self.requires("up-client-zenoh-cpp/0.1")
        self.requires("protobuf/3.21.12" + ("@cross/cross" if self.options.build_cross_compiling else ""))
            

    def imports(self):
        if self.options.build_testing:
            self.copy("*.so*", dst="lib", keep_path=False)
