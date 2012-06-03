#define BUILDING_NODE_EXTENSION
#include <node.h>
#include <mcnet.h>

#include "parser.h"

using namespace v8;

mcnet::Parser::Parser() {
  settings.on_packet = mcnet::Parser::on_packet;
  settings.on_error = mcnet::Parser::on_error;
}

mcnet::Parser::~Parser() {
}

void mcnet::Parser::Init(Handle< Object > target) {
  Local< FunctionTemplate > tpl = FunctionTemplate::New(New);

  tpl->SetClassName(String::NewSymbol("Parser"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  tpl->PrototypeTemplate()->Set(String::NewSymbol("execute"), FunctionTemplate::New(Execute)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("EAGAIN"), Number::New(MCNET_EAGAIN));
  tpl->PrototypeTemplate()->Set(String::NewSymbol("EINVALID"), Number::New(MCNET_EINVALID));

  Persistent< Function > constructor = Persistent< Function >::New(tpl->GetFunction());
  target->Set(String::NewSymbol("Parser"), constructor);
}

Handle< Value > mcnet::Parser::New(const Arguments& args) {
  HandleScope scope;

  Parser* obj = new Parser();
  obj->Wrap(args.This());

  obj->parser.data = (void*)obj;

  return args.This();
}

Handle< Value > mcnet::Parser::Execute(const Arguments& args) {
  HandleScope scope;

  mcnet::Parser* parser = ObjectWrap::Unwrap< mcnet::Parser >(args.This());

  size_t nparsed = mcnet_parser_execute(&(parser->parser), &(parser->settings), NULL, 0);

  return scope.Close(Number::New(nparsed));
}

void mcnet::Parser::on_packet(mcnet_parser_t* parser, mcnet_packet_t* packet) {
//  mcnet::Parser* self = (mcnet::Parser*)(parser->data);
}

void mcnet::Parser::on_error(mcnet_parser_t* parser, int err) {
//  mcnet::Parser_t* self = (mcnet::Parser_t*)(parser->data);
}
