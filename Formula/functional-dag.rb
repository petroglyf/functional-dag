require "formula"

class FunctionalDag < Formula
  desc "A library for building functional dags"
  homepage "https://github.com/petrogly-ph/functional-dag"
  url "https://github.com/petrogly-ph/functional-dag/archive/refs/tags/brew.tar.gz"
  sha256 "38775c8234d598e1e1f099861a19e000564a08c66ff15643bbf6cc8e5d707ed1"
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
