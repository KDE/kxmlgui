from conans import ConanFile, CMake
import yaml
import re
import os.path


def getVersion():
    if(os.path.exists("CMakeLists.txt")):
        regx = re.compile(r"^set\(.*VERSION\s(\"|')[0-9.]+(\"|')\)")
        with open("CMakeLists.txt") as f:
            for line in f:
                if regx.match(line):
                    version = re.search("\"[0-9\.]+\"", line)
                    version = version.group().replace("\"", "")
                    return version
    return None


def getMetaField(field):
    if(os.path.exists("metainfo.yaml")):
        with open("metainfo.yaml") as f:
            metainfo = yaml.load(f.read())
        return metainfo[field]
    return None


class KXmlGui(ConanFile):
    name = getMetaField('name')
    version = getVersion()
    license = getMetaField('license')
    url = getMetaField('url')
    description = getMetaField('description')

    settings = "os", "compiler", "build_type", "arch"

    default_options = (
        "qt:qtx11extras = True",
        "qt:qtdeclarative = True",
        "qt:with_pq=False"
    )
    requires = (
        "qt/[>=5.11.1]@bincrafters/stable",     # Done
        "extra-cmake-modules/[>=5.61.0]@kde/testing",   # Done

        "kattica/[>=5.61.0]@kde/testing",
        "kconfig/[>=5.61.0]@kde/testing",       # Done
        "kconfigwidgets/[>=5.61.0]@kde/testing",
        "kcoreaddons/[>=5.61.0]@kde/testing",   # Done
        "kglobalaccel/[>=5.61.0]@kde/testing"
        "ki18n/[>=5.61.0]@kde/testing",         # Done
        "kiconthemes/[>=5.61.0]@kde/testing",
        "kitemviews/[>=5.61.0]@kde/testing",
        "ktextwidgets/[>=5.61.0]@kde/testing",
        "kwidgetaddons/[>=5.61.0]@kde/testing",
        "kwindowsystem/[>=5.61.0]@kde/testing",  # Done
    )

    generators = "cmake"
    scm = {
        "type": "git",
        "url": "auto",
        "revision": "auto"
    }

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
        cmake.install()

    def package_info(self):
        self.cpp_info.resdirs = ["share"]
