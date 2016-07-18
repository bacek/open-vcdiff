// Copyright (c) 2016 Yandex LLC. All rights reserved.
// Author: Vasily Chekalkin <bacek@yandex-team.ru>

#include "google/vcdecoder.h"

#include <emscripten/bind.h>
using namespace emscripten;

class VCDiffStreamingDecoder {
public:
  explicit VCDiffStreamingDecoder(const std::string &dict);

  std::string decodeChunk(const std::string& chunk);
  bool finishDecoding();
private:

  std::string dict_;
  open_vcdiff::VCDiffStreamingDecoder decoder_;

  bool is_ok_;
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

