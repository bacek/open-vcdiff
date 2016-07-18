// Copyright (c) 2016 Yandex LLC. All rights reserved.
// Author: Vasily Chekalkin <bacek@yandex-team.ru>

#include "google/vcdecoder.h"

#include <emscripten/bind.h>
using namespace emscripten;

// 
// This is simple wrapper for open_vcdiff::VCDiffStreamingDecoder to simplify
// emscriptened version. Just because emscripten does support std::string
// mapping out of the box and memory model is quite different between JS and C++
// worlds.
//
// To compile vcdiff.js from current source you'll need to:
// 1. Install emscripten SDK from emscripten.org
// 2. Run "emconfigure ./configure --disable-shared"
// 3. Run "emmake make -k". -k is for variant when you don't have gflags/gtest
// checked out
// 4. Run "em++ --bind -Oz --closure 1 -s FORCE_FILESYSTEM=0  -s
// BUILD_AS_WORKER=0   -o vcdiff.js ems_binding.cc -Isrc .libs/libvcddec.a
// .libs/libvcdcom.a"
// 5. Use vcdiff.js
//
// Example of usage:
//
// test.js:
//
// "use strict";
// 
// var Module = require("./vcdiff.js")
//   , util = require("util");
// 
// var decoder = new Module.VCDiffStreamingDecoder("This is an Javascript dictionary\n");
// 
// var encodedHex = [
//   "d6", "c3", "c4", "53",
//   "00", "05", "21", "00", "0d", "21", "00", "00", "03", "00", "8c", "d2", "c4",
//   "98", "1a", "73", "21", "00"
// ];
// 
// var encoded = new Uint8Array(encodedHex.map(d => parseInt(d, 16)));
// 
// var res = decoder.decodeChunk(encoded);
// console.log(util.inspect(res));
// 
// var f = decoder.finish();
// console.log(f);
// 
// Run:
// $ node test.js
// 'This is an Javascript dictionary\n'
// true
// 
//
class VCDiffStreamingDecoder {
public:
  explicit VCDiffStreamingDecoder(const std::string &dict);

  std::string decodeChunk(const std::string& chunk);
  bool finishDecoding();
private:

  std::string dict_;
  open_vcdiff::VCDiffStreamingDecoder decoder_;
};

VCDiffStreamingDecoder::VCDiffStreamingDecoder(const std::string &dict)
    : dict_(dict) {
  decoder_.StartDecoding(dict_.data(), dict_.length());
};

std::string VCDiffStreamingDecoder::decodeChunk(const std::string& chunk) {
  std::string res;
  decoder_.DecodeChunk(chunk.data(), chunk.length(), &res);
  return res;
}

bool VCDiffStreamingDecoder::finishDecoding() {
  return decoder_.FinishDecoding();
}

EMSCRIPTEN_BINDINGS(open_vcdiff) {
  class_<VCDiffStreamingDecoder>("VCDiffStreamingDecoder")
    .constructor<std::string>()
    .function("decodeChunk", &VCDiffStreamingDecoder::decodeChunk)
    .function("finish", &VCDiffStreamingDecoder::finishDecoding)
    ;
}

