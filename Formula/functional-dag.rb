require "formula"

class FunctionalDag < Formula
  desc "A library for building functional dags"
  homepage "https://github.com/petrogly-ph/functional-dag"
  url "https://github.com/petrogly-ph/functional-dag/archive/refs/tags/brew.tar.gz"
  sha256 "6f19bce91912b14324aa469855f8516bce9e797b014ce0a7ada3f79a2ca90627"
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

  # test do
  #   system "false"
  # end
end
