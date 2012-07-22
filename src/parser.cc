#include <cstring>
#include <iostream>

#define BUILDING_NODE_EXTENSION
#include <node.h>
#include <node_buffer.h>
#include <mcnet.h>

#include "parser.h"

using namespace v8;
using namespace node;

mcnet::Parser::Parser(char parser_type) {
  parser.type = parser_type;
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
  tpl->PrototypeTemplate()->Set(String::NewSymbol("TYPE_CLIENT"), Number::New(MCNET_PARSER_TYPE_CLIENT));
  tpl->PrototypeTemplate()->Set(String::NewSymbol("TYPE_SERVER"), Number::New(MCNET_PARSER_TYPE_SERVER));

  Persistent< Function > constructor = Persistent< Function >::New(tpl->GetFunction());
  target->Set(String::NewSymbol("Parser"), constructor);
}

Handle< Value > mcnet::Parser::New(const Arguments& args) {
  HandleScope scope;

  int parser_type;
  if (args.Length() > 0 && args[0]->IsNumber()) {
    parser_type = args[0]->NumberValue();
  } else {
    parser_type = MCNET_PARSER_TYPE_SERVER;
  }

  Parser* obj = new Parser(parser_type);
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
  Local< Object > name##_metadata = Object::New(); \
  mcnet_metadata_parser_t name##_parser; \
  name##_parser.on_error = NULL; \
  name##_parser.on_complete = NULL; \
  name##_parser.on_entry = mcnet::Parser::on_metadata_entry; \
  name##_parser.data = &name##_metadata; \
  mcnet_metadata_parser_parse(&name##_parser, pkt->name, pkt->name##_len); \
  name##_parser.data = NULL; \
  object->Set(String::New(#name), name##_metadata);
#define SLOT(name) \
  Local< Object > name##_slot = Object::New(); \
  mcnet_slot_parser_t name##_parser; \
  name##_parser.on_error = NULL; \
  name##_parser.on_complete = mcnet::Parser::on_slot; \
  name##_parser.data = &name##_slot; \
  mcnet_slot_parser_parse(&name##_parser, pkt->name, pkt->name##_len); \
  name##_parser.data = NULL; \
  object->Set(String::New(#name), name##_slot);
#define SLOTS(name, count) \
  Local< Array > name##_slots = Array::New(pkt->count); \
  mcnet_slot_parser_t name##_parser; \
  name##_parser.on_error = NULL; \
  name##_parser.on_complete = mcnet::Parser::on_slot; \
  int name##_nparsed = 0, name##_i = 0; \
  while (name##_nparsed < pkt->name##_len) { \
    Local< Object > name##_slot = Object::New(); \
    name##_parser.data = &name##_slot; \
    name##_nparsed += mcnet_slot_parser_parse(&name##_parser, pkt->name + name##_nparsed, pkt->name##_len - name##_nparsed); \
    name##_parser.data = NULL; \
    name##_slots->Set(name##_i++, name##_slot); \
  } \
  object->Set(String::New(#name), name##_slots);

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

void mcnet::Parser::on_metadata_entry(mcnet_metadata_parser_t* parser, mcnet_metadata_entry_t* entry) {
  Local< Object >* obj = reinterpret_cast< Local< Object >* >(parser->data);

  switch (entry->type) {
    case MCNET_METADATA_TYPE_BYTE: {
      mcnet_metadata_entry_byte_t* ent = reinterpret_cast< mcnet_metadata_entry_byte_t* >(entry);
      (*obj)->Set(ent->index, Number::New(ent->data));
      break;
    }
    case MCNET_METADATA_TYPE_SHORT: {
      mcnet_metadata_entry_short_t* ent = reinterpret_cast< mcnet_metadata_entry_short_t* >(entry);
      (*obj)->Set(ent->index, Number::New(ent->data));
      break;
    }
    case MCNET_METADATA_TYPE_INT: {
      mcnet_metadata_entry_int_t* ent = reinterpret_cast< mcnet_metadata_entry_int_t* >(entry);
      (*obj)->Set(ent->index, Number::New(ent->data));
      break;
    }
    case MCNET_METADATA_TYPE_FLOAT: {
      mcnet_metadata_entry_float_t* ent = reinterpret_cast< mcnet_metadata_entry_float_t* >(entry);
      (*obj)->Set(ent->index, Number::New(ent->data));
      break;
    }
    case MCNET_METADATA_TYPE_STRING16: {
      mcnet_metadata_entry_string16_t* ent = reinterpret_cast< mcnet_metadata_entry_string16_t* >(entry);
      Buffer* buffer = Buffer::New(ent->data_length * 2);
      memcpy(Buffer::Data(buffer), ent->data, ent->data_length * 2);
      Handle< Value > args[3] = { buffer->handle_, Integer::New(ent->data_length * 2), Integer::New(0) };
      Local< Object > global = Context::GetCurrent()->Global();
      Local< Function > buffer_constructor = Local< Function >::Cast(global->Get(String::New("Buffer")));
      (*obj)->Set(ent->index, buffer_constructor->NewInstance(3, args));
      break;
    }
    case MCNET_METADATA_TYPE_SLOT: {
      mcnet_metadata_entry_sbs_t* ent = reinterpret_cast< mcnet_metadata_entry_sbs_t* >(entry);
      Local< Object > object = Object::New();
      object->Set(String::New("id"), Number::New(ent->id));
      object->Set(String::New("count"), Number::New(ent->count));
      object->Set(String::New("damage"), Number::New(ent->damage));
      (*obj)->Set(ent->index, object);
      break;
    }
    case MCNET_METADATA_TYPE_INTS: {
      mcnet_metadata_entry_iii_t* ent = reinterpret_cast< mcnet_metadata_entry_iii_t* >(entry);
      Local< Array > array = Array::New(3);
      array->Set(0, Number::New(ent->data[0]));
      array->Set(1, Number::New(ent->data[1]));
      array->Set(2, Number::New(ent->data[2]));
      (*obj)->Set(ent->index, array);
      break;
    }
  }
}

void mcnet::Parser::on_slot(mcnet_slot_parser_t* parser, mcnet_slot_t* slot) {
  Local< Object >* obj = reinterpret_cast< Local< Object >* >(parser->data);

  (*obj)->Set(String::New("item"), Number::New(slot->item));
  (*obj)->Set(String::New("count"), Number::New(slot->count));
  (*obj)->Set(String::New("meta"), Number::New(slot->meta));

  Buffer* buffer = Buffer::New(slot->data_len);
  memcpy(Buffer::Data(buffer), slot->data, slot->data_len);
  Handle< Value > args[3] = { buffer->handle_, Integer::New(slot->data_len), Integer::New(0) };
  Local< Object > global = Context::GetCurrent()->Global();
  Local< Function > buffer_constructor = Local< Function >::Cast(global->Get(String::New("Buffer")));
  (*obj)->Set(String::New("data"), buffer_constructor->NewInstance(3, args));
}
