class FunctionalDag < Formula
  desc "This library is used for building functional dags"
  homepage "https://github.com/petrogly-ph/functional-dag"
  url "https://github.com/petroglyf/functional-dag/archive/refs/tags/brew-v1.0.tar.gz"
  version "1.0"
  license "MIT"
  sha256 "e6a91796675e4a247b5e68fa24167d066bac0e340a6e62d5ec49050f7d52f33e"

  depends_on "meson" => :build
  depends_on "catch2" => :test
  depends_on "ninja" => :build
  depends_on "flatbuffers" => :build

  def install
    system "meson", "setup", "build", *std_meson_args
    system "ninja", "-C", "build", "libfunctional_dag.dylib"
    system "meson", "install", "-C", "build"
  end

  # Testing is performed manually until v1.0
  test do
    # `test do` will create, run in and delete a temporary directory.
    #
    # This test will fail and we won't accept that! For Homebrew/homebrew-core
    # this will need to be a test that verifies the functionality of the
    # software. Run the test with `brew test functional-dag`. Options passed
    # to `brew install` such as `--HEAD` also need to be provided to `brew test`.
    #
    # The installed folder is not in the path, so use the entire path to any
    # executables being tested: `system bin/"program", "do", "something"`.
    # system "meson", "test"
    system "false"
  end
end
