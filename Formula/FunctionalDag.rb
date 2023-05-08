require "formula"

class Functionaldag < Formula
  desc "A library for building functional dags"
  homepage "https://github.com/petrogly-ph/functional-dag"
  url "https://github.com/petrogly-ph/functional-dag/archive/refs/tags/brew.tar.gz"
  sha256 "3c375b67fe8f4785fb65ba8e1bc2e5f8fe01d3552781921d376ce1d664a5091c"
  license "MIT"
  version "1.0"

  # bottle do
  #   cellar :any
  #   sha1 "5466fbee57b366a41bbcec814614ee236e39bed8" => :yosemite
  #   sha1 "bde270764522e4a1d99767ca759574a99485e5ac" => :mavericks
  #   sha1 "e77d0e5f516cb41ac061e1050c8f37d0fb65b796" => :mountain_lion
  # end

  depends_on "cmake" => :build
  depends_on "catch2" => :test
  depends_on "jsoncpp" => :build

  def install
    # ENV.cxx20 if build.cxx20?
    mkdir "filterdag-build" do
      args = std_cmake_args
      
      system "cmake", "..", *args
      system "make", "install"
    end


    # ENV.deparallelize
    # system "./configure", *std_configure_args, "--disable-silent-rules"
    # system "cmake", "-S", ".", "-B", "build", *std_cmake_args
    # system "make", "install"
  end

  # test do
  #   system "false"
  # end
end
