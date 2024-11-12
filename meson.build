project(
    'functional-dag',
    'cpp',
    default_options: [
        'c_std=c20',
        'cpp_std=c++20',
        'warning_level=3',
        'werror=true',
    ],
    version: '0.5',
    license: 'MIT',
    license_files: 'LICENSE',
)

if build_machine.subsystem() == 'macos'
    homebrew_cellar = run_command('brew', '--cellar', check: true).stdout().strip()
    install_cellar = homebrew_cellar + '/' + meson.project_name() + '/' + meson.project_version()
    include_install_dir = install_cellar + '/include'
    shared_lib_dir = install_cellar + '/lib'
    pkg_config_install_dir = shared_lib_dir + '/pkgconfig'
else
    include_install_dir = '/usr/include'
    shared_lib_dir = '/usr/lib'
    pkg_config_install_dir = '/usr/share/pkgconfig'
endif

##########################################
####### Find the needed packages #########
##########################################
catch_dep = dependency('catch2-with-main')
jsoncpp_dep = dependency('jsoncpp')
flatbuffers_dep = dependency('flatbuffers')
pkg_fn = import('pkgconfig')
fs_dep = import('fs')

########################################
####### Building specification #########
########################################
functional_dag_lib = shared_library(
    'functional_dag',
    ['src/functional_dag/libutils.cpp'],
    include_directories: ['include/'],
    dependencies: jsoncpp_dep,
    install: true,
    install_dir: shared_lib_dir,
)

########################################
####### Build the testing files ########
########################################
dag_tests = executable(
    'dag_tests',
    'test/functional_dag/dag_tests.cpp',
    include_directories: ['include/'],
    dependencies: [catch_dep],
)
lib_tests = executable(
    'lib_tests',
    'test/functional_dag/lib_tests.cpp',
    include_directories: ['include/'],
    dependencies: [jsoncpp_dep, catch_dep],
    link_with: [functional_dag_lib],
)
test('dag_tests', dag_tests)
# test('lib_tests', lib_tests)

########################################
####### Lint command (optional) ########
########################################
clangtidy = find_program('clang-tidy', required: false)
if clangtidy.found()
    run_target(
        'tidy',
        command: [
            clangtidy,
            #'--checks=*',
            '-p',
            meson.project_source_root(),
        ] + ['../src/functional_dag/libutils.cpp', '--', '-Iinclude/'],
    )
endif

########################################
####### Install specification ##########
########################################
install_subdir('include/functional_dag', install_dir: include_install_dir)  #homebrew_cellar+'/'+install_dir
pkg_fn.generate(
    functional_dag_lib,
    description: 'A directed acyclic graph that runs a function on each node.',
    install_dir: pkg_config_install_dir,
)

########################################
##### Packaging specifications #########
########################################
if build_machine.subsystem() != 'macos'
    arch_id = get_option('ARCH_ID')

    control_cp = fs_dep.copyfile(
        'debian/control',
        install: true,
        install_dir: '../../DEBIAN/',
    )
    deb_package_name = meson.project_name() + '-' + meson.project_version() + '.' + arch_id + '.deb'
    mytarget = custom_target(
        'debpackage',
        # input: 'DEBIAN/control',
        output: deb_package_name,
        command: ['dpkg-deb', '-b', 'install/', '@OUTPUT@'],
    )
endif
