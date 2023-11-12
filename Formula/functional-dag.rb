require "formula"

class FunctionalDag < Formula
  desc "A library for building functional dags"
  homepage "https://github.com/petrogly-ph/functional-dag"
  url "https://github.com/petrogly-ph/functional-dag/archive/refs/tags/brew-v0.5.tar.gz"
  sha256 "284fa1af920934c4660bbc7954e6854bf485c20f5ba85f8b5f73d0a1a7387f72"
  license "MIT"
  version "0.5"

  # Don't yet support bottles! 
  # bottle do
  #   cellar :any
  #   sha1 "5466fbee57b366a41bbcec814614ee236e39bed8" => :ventura
  # end

  depends_on "cmake" => :build
  depends_on "catch2" => :test
  depends_on "jsoncpp" => :build

  def install
    mkdir "filterdag-build" do
      args = std_cmake_args
      
      system "cmake", "..", *args
      system "make", "install"
    end
  end

  # Testing is performed manually until v1.0
  # test do
  #   system "false"
  # end
end
