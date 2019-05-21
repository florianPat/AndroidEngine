# Sets the minimum version of CMake required to build the native library.
cmake_minimum_required(VERSION 3.4.1)

add_library(freetype2
        STATIC
        IMPORTED )
set_target_properties( # Specifies the target library.
        freetype2

        # Specifies the parameter you want to define.
        PROPERTIES IMPORTED_LOCATION

        # Provides the path to the library you want to import.
        # NOTE: Need to add currentSourceDir here because a relative one does not work
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/freetype2/build/${ANDROID_ABI}/libfreetype2.a )
include_directories( libs/freetype2/include )

add_library(libpng
            STATIC
            IMPORTED )
set_target_properties(libpng

        PROPERTIES IMPORTED_LOCATION

        ${CMAKE_CURRENT_SOURCE_DIR}/libs/libpng/build/${ANDROID_ABI}/liblibpng.a )
include_directories( libs/libpng )

# Export ANativeActivity_onCreate(),
# Refer to: https://github.com/android-ndk/ndk/issues/381.
set(CMAKE_SHARED_LINKER_FLAGS
        "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

#NOTE: For building the static lib
add_library( androidEngine
             STATIC
             src/main/cpp/TranslationUnit.cpp )

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.
add_library( # Sets the name of the library.
             native-activity

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
             src/main/cpp/android_native_app_glue.c
             src/main/cpp/Actor.cpp
             src/main/cpp/Animation.cpp
             src/main/cpp/Array.cpp
             src/main/cpp/AssetManager.cpp
             src/main/cpp/Benchmark.cpp
             src/main/cpp/CircleShape.cpp
             src/main/cpp/Clock.cpp
             src/main/cpp/EventLevelReload.cpp
             src/main/cpp/EventManager.cpp
             src/main/cpp/Font.cpp
             src/main/cpp/GameObjectManager.cpp
             src/main/cpp/GLUtils.cpp
             src/main/cpp/GraphicsOGL2.cpp
             src/main/cpp/GraphicsOGLIniter.cpp
             src/main/cpp/Ifstream.cpp
             src/main/cpp/IndexBuffer.cpp
             src/main/cpp/InkscapeAnimation.cpp
             src/main/cpp/InscapeAnimationElement.cpp
             src/main/cpp/Level.cpp
             src/main/cpp/main.cpp
             src/main/cpp/MainLevel.cpp
             src/main/cpp/Physics.cpp
             src/main/cpp/RectangleShape.cpp
             src/main/cpp/RenderTexture.cpp
             src/main/cpp/Shader.cpp
             src/main/cpp/Sound.cpp
             src/main/cpp/Sprite.cpp
             src/main/cpp/String.cpp
             src/main/cpp/Texture.cpp
             src/main/cpp/TextureAtlas.cpp
             src/main/cpp/TiledMap.cpp
             src/main/cpp/Utils.cpp
             src/main/cpp/Vector.cpp
             src/main/cpp/VertexArray.cpp
             src/main/cpp/VertexBuffer.cpp
             src/main/cpp/VertexLayout.cpp
             src/main/cpp/View.cpp
             src/main/cpp/Window.cpp )

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
                       native-activity

                       log
                       android
                       EGL
                       GLESv2
                       OpenSLES
                       libpng
                       freetype2 )
                       #NOTE: For building the static lib
                       #androidEngine )