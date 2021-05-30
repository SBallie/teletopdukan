require 'formula'

class I386ElfBinutils < Formula
  homepage 'http://gcc.gnu.org'
  url 'http://ftp.gnu.org/gnu/binutils/binutils-2.23.tar.gz'
  sha1 '470c388c97ac8d216de33fa397d7be9f96c3fe04'

  depends_on 'gcc49' => :build

  def install
    ENV['CC'] = '/usr/local/bin/gcc-4.9'
    ENV['CXX'] = '/usr/local/bin/g++-4.9'
    ENV['CPP'] = '/usr/local/bin/cpp-4.