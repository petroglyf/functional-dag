class FunctionalDag < Formula
  desc "This library is used for building functional dags"
  homepage "https://github.com/petrogly-ph/functional-dag"
  url "https://github.com/petrogly-ph/functional-dag/archive/refs/tags/brew-v1.0rc1.tar.gz"
  version "1.0rc1"
  license "MIT"
  sha256 "284fa1af92093a4c4660bbc7954e6854bf485c20f5ba85f8b5f73d0a1a7387f72"

  # Don't yet support bottles!
  bottle do
    cellar :any
    sha1 "5466fbee57b366a41bbcec814614ee236e39bed8" => :sequoia
  end

  depends_on "meson" => :build
  depends_on "catch2" => :test

  def install
    build_directory = "filterdag-build"
    args = std_meson_args
    
    mkdir build_directory do
      system "meson", "setup", *args, ".."
      system "meson", "install"
    end
  end

  # Testing is performed manually until v1.0
  test do
    system "meson", "test"
  end
end
