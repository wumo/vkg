from conans import ConanFile, CMake, tools


class vkgConan(ConanFile):
    name = "vkg"
    version = "0.0.9"
    settings = "os", "compiler", "build_type", "arch"
    requires = (
        "vulkan-headers/1.2.148@wumo/stable",
        "vma/2.3.0@wumo/stable",
        "glfw/3.4@wumo/stable",
        "spirv-cross/20200917",
        "glm/0.9.9.8",
        "stb/20200203",
        "tinygltf/2.5.0",
        "par_lib/master@wumo/stable",
        "magic_get/2.0.1@wumo/stable",
        # "bullet3/3.07"
    )
    build_requires = ("glslvk/0.0.2@wumo/stable")
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
        "shared": True,
        "glfw:shared": False,
    }

    def configure_cmake(self):
        cmake = CMake(self)
        cmake.definitions["BUILD_TEST"] = False
        cmake.definitions["BUILD_SHARED"] = self.options.shared
        cmake.definitions["ENABLE_VALIDATION_LAYER"] = False
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
        self.cpp_info.defines = ["VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1"]
        if self.options.shared:
            self.cpp_info.defines.append("VULKAN_HPP_STORAGE_SHARED=1")
