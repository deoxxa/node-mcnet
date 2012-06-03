#define BUILDING_NODE_EXTENSION
#include <node.h>

#include "parser.h"

using namespace v8;

void InitAll(Handle<Object> target) {
  mcnet::Parser::Init(target);
}

NODE_MODULE(mcnet, InitAll)
