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
// 2. Run "emcmake -Dvcdiff_build_tests=OFF -Dvcdiff_build_exec=OFF -DBUILD_SHARED_LIBS=OFF -Dvcdiff_build_js=ON"
// 3. Run "emmake make".
// 4. Use vcdiff.js
//
// NB: If you are going to use it in browser WebWorker you can't use -O2 and
// above. It wouldn't work for whatever reason. Use "-O1 -closure 2" for
// minification. "TypeError: Module.VCDiffStreamingDecoder is not a constructor(…)"
//
// However it will work in node.js with higher optimization options.
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

