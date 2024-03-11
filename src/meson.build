project(
  'meson test',
  'cpp',
default_options : ['c_std=c11', 'cpp_std=c++20']
)

currentdir = meson.current_source_dir()
incdir = include_directories('./graphics','./library','./gui','./audio', 'VNCore')


cc = meson.get_compiler('cpp')
opengl = dependency('gl')
threads = dependency('threads')

if(host_machine.system() == 'windows')
  glfw = cc.find_library('glfw3', static : true)
  openal = cc.find_library('OpenAL32')

else
  glfw = cc.find_library('glfw')
  openal = cc.find_library('openal')
endif

sources = files(
'main.cpp',

'graphics/Shader.cpp',
'graphics/VAO.cpp',
'graphics/View.cpp',
'graphics/Texture.cpp',
'graphics/FBO.cpp',
'graphics/Armature.cpp',
'graphics/ArmatureConstraints.cpp',
'graphics/Mesh.cpp',
'graphics/DebugDraw.cpp',

'audio/Audio.cpp',

'library/glad.cpp',
'library/stb_image.cpp',
'library/stb_vorbis.cpp',

'gui/FontInfo.cpp',
'gui/Text.cpp',
'gui/GUI.cpp',
'gui/Menu.cpp',
'gui/Element.cpp',
'gui/ElementSet.cpp',

'VNCore/VNInterpreter.cpp',
'VNCore/VNVariable.cpp',
'VNCore/Window.cpp',
'VNCore/VNCompiledFile.cpp',
'VNCore/VNOperation.cpp',
'VNCore/VNAssetManager.cpp',
'VNCore/VNDebug.cpp',
'VNCore/ExpressionParser.cpp',

'VNOperationDefs/OperationDefs.cpp',
'VNOperationDefs/OperationsArithmetic.cpp',
'VNOperationDefs/OperationsArmature.cpp',
'VNOperationDefs/OperationsAudio.cpp',
'VNOperationDefs/OperationsDialogue.cpp',
'VNOperationDefs/OperationsControl.cpp',
'VNOperationDefs/OperationsGUI.cpp',
'VNOperationDefs/OperationsImage.cpp',
'VNOperationDefs/OperationsModel.cpp',
'VNOperationDefs/OperationsObject.cpp',
'VNOperationDefs/OperationsShader.cpp',
'VNOperationDefs/OperationsString.cpp',
'VNOperationDefs/OperationsView.cpp'

)

if(host_machine.system() == 'windows')
  executable('exec',sources, include_directories : incdir, dependencies : [glfw, opengl, openal,threads], override_options : ['std=c++20'])
else
  executable('exec',sources, include_directories : incdir, dependencies : [glfw, opengl, openal, threads], override_options : ['std=c++20'])

endif

