#include <cstring>

#define BUILDING_NODE_EXTENSION
#include <node.h>
#include <node_buffer.h>
#include <mcnet.h>

#include "parser.h"

using namespace v8;
using namespace node;

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

  return args.This();
}

Handle< Value > mcnet::Parser::Execute(const Arguments& args) {
  HandleScope scope;

  mcnet::Parser* parser = ObjectWrap::Unwrap< mcnet::Parser >(args.This());

  Local< Object > buffer = Local< Object >::Cast(args[0]);

  Handle< Object > obj = args.This();
  parser->parser.data = (void*)&obj;
  size_t nparsed = mcnet_parser_execute(&(parser->parser), &(parser->settings), reinterpret_cast< uint8_t* >(Buffer::Data(buffer)), Buffer::Length(buffer));
  parser->parser.data = NULL;

  return scope.Close(Number::New(nparsed));
}

void mcnet::Parser::on_packet(mcnet_parser_t* parser, mcnet_packet_t* packet) {
  Handle< Object >* obj = (Handle< Object >*)(parser->data);

  Local< Object > object = Object::New();

  Local< Object > global = Context::GetCurrent()->Global();
  Local< Function > buffer_constructor = Local< Function >::Cast(global->Get(String::New("Buffer")));

  switch (packet->pid) {
#include "cases.cc"
  }

  Handle< Value > argv[2] = {
    String::New("packet"),
    object
  };

  MakeCallback(*obj, "emit", 2, argv);
}

void mcnet::Parser::on_error(mcnet_parser_t* parser, int err) {
  if (err == MCNET_EAGAIN) {
    return;
  }

  Handle< Object >* obj = (Handle< Object >*)(parser->data);

  Handle< Value > argv[2] = {
    String::New("error"),
    Number::New(err)
  };

  MakeCallback(*obj, "emit", 2, argv);
}
