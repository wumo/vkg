from conans import ConanFile, CMake, tools

class vkgConan(ConanFile):
    name = "vkg"
    version = "0.0.1"
    settings = "os", "compiler", "build_type", "arch"
    requires = (
        "vulkan-headers/1.2.148@wumo/stable",
        "vma/2.3.0@wumo/stable",
        "glfw/3.4@wumo/stable",
        "spirv-cross/20200519",
        "glm/0.9.9.8",
        "stb/20200203",
        "tinygltf/2.2.0",
        "par_lib/master@wumo/stable",
    )
    build_requires = ("file2header/1.0.7@wumo/stable")
    generators = "cmake"
    scm = {
        "type": "git",
        "subfolder": name,
        "url": "auto",
        "revision": "auto"
    }
    options = {
        "shared": [True, False],
    }
    default_options = {
        "shared": True
    }

    def configure_cmake(self):
        cmake = CMake(self)
        cmake.definitions["BUILD_TEST"] = False
        cmake.definitions["BUILD_SHARED"] = self.options.shared
        cmake.configure(source_folder=self.name)
        return cmake

    def build(self):
        cmake = self.configure_cmake()
        cmake.build()

    def imports(self):
        self.copy("*.dll", dst="bin", src="bin")
        self.copy("*.dll", dst="bin", src="lib")
        self.copy("*.dylib", dst="bin", src="lib")
        self.copy("*.pdb", dst="bin", src="bin")

    def package(self):
        cmake = self.configure_cmake()
        cmake.install()
        self.copy("*.h", dst="include", src=f"{self.name}/src")
        self.copy("*.hpp", dst="include", src=f"{self.name}/src")

    def package_info(self):
        self.cpp_info.libs = tools.collect_libs(self)
