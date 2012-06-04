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

#define PACKET(id, code) case 0x##id: { \
  mcnet_packet_##id##_t* pkt = reinterpret_cast< mcnet_packet_##id##_t* >(packet); \
  BYTE(pid) \
  code \
  break; \
}

#define CODE(data)
#define BOOL(name) object->Set(String::New(#name), Boolean::New(pkt->name ? true : false));
#define NUMBER(name) object->Set(String::New(#name), Number::New(pkt->name));
#define BYTE(name) NUMBER(name)
#define UBYTE(name) NUMBER(name)
#define SHORT(name) NUMBER(name)
#define USHORT(name) NUMBER(name)
#define INT(name) NUMBER(name)
#define LONG(name) NUMBER(name)
#define FLOAT(name) NUMBER(name)
#define DOUBLE(name) NUMBER(name)
#define STRING8(name) BLOB(name, name##_len)
#define STRING16(name) BLOB(name, name##_len * 2)
#define BLOB(name, length) \
  Buffer* name##_buffer = Buffer::New(pkt->length); \
  memcpy(Buffer::Data(name##_buffer), pkt->name, pkt->length); \
  Handle< Value > name##_args[3] = { name##_buffer->handle_, Integer::New(pkt->length), Integer::New(0) }; \
  object->Set(String::New(#name), buffer_constructor->NewInstance(3, name##_args));
#define METADATA(name) \
  object->Set(String::New(#name), String::New("metadata placeholder"));
#define SLOT(name) \
  object->Set(String::New(#name), String::New("slot placeholder"));
#define SLOTS(name, count) \
  object->Set(String::New(#name), String::New("slot array placeholder"));

  switch (packet->pid) {
PACKETS
  }

#undef CODE
#undef BOOL
#undef NUMBER
#undef BYTE
#undef UBYTE
#undef SHORT
#undef USHORT
#undef INT
#undef LONG
#undef FLOAT
#undef DOUBLE
#undef STRING8
#undef STRING16
#undef BLOB
#undef METADATA
#undef SLOT
#undef SLOTS

#undef PACKET

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
