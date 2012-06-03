#ifndef NODE_MCNET_PARSER_H
#define NODE_MCNET_PARSER_H

#include <node.h>
#include <mcnet.h>

using namespace v8;

namespace mcnet {
  typedef class Parser : public node::ObjectWrap {
  public:
    static void Init(Handle<Object> target);

  private:
    Parser();
    ~Parser();

    mcnet_parser_t parser;
    mcnet_parser_settings_t settings;

    static Handle<Value> New(const Arguments& args);
    static Handle<Value> Execute(const Arguments& args);
    static void on_packet(mcnet_parser_t* parser, mcnet_packet_t* packet);
    static void on_error(mcnet_parser_t* parser, int err);
  } Parser_t;
}

#endif