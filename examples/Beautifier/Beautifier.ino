#include <jsonlite.h>

#define MAX_JSON_DEPTH (16)

typedef struct context {
    jsonlite_parser parser;
    jsonlite_builder builder;
} context;

static void event_occurred(jsonlite_callback_context *ctx, jsonlite_event event) {
    context *c = (context *)ctx->client_state;
    switch (event) {
    case jsonlite_event_finished:
        break;
    case jsonlite_event_object_start:
        jsonlite_builder_object_begin(c->builder);
        break;
    case jsonlite_event_object_end:
        jsonlite_builder_object_end(c->builder);
        break;
    case jsonlite_event_array_start:
        jsonlite_builder_array_begin(c->builder);
        break;
    case jsonlite_event_array_end:
        jsonlite_builder_array_end(c->builder);
        break;
    default:
        break;
    }
}

static void token_callback(jsonlite_callback_context *ctx, jsonlite_token *t) {
    context *c = (context *)ctx->client_state;
    jsonlite_token_type type = t->type & jsonlite_token_type_mask;
    switch (type) {
    case jsonlite_token_null:
        jsonlite_builder_null(c->builder);
        break;
    case jsonlite_token_true:
        jsonlite_builder_true(c->builder);
        break;
    case jsonlite_token_false:
        jsonlite_builder_false(c->builder);
        break;
    case jsonlite_token_key:
        jsonlite_builder_raw_key(c->builder, t->start, t->end - t->start);
        break;
    case jsonlite_token_number:
        jsonlite_builder_raw_value(c->builder, t->start, t->end - t->start);
        break;
    case jsonlite_token_string:
        jsonlite_builder_raw_string(c->builder, t->start, t->end - t->start);
        break;
    default:
        break;
    }
}

char json[] = "{\"a\":null,\"b\":[1,2,3],\"c\":true,\"d\":{\"a\":1,\"b\":[]},\"e\":false,\"f\":[\"a\",\"a\",\"a\"]}";
uint8_t parser_memory[jsonlite_parser_estimate_size(MAX_JSON_DEPTH)];
uint8_t builder_memory[jsonlite_builder_estimate_size(MAX_JSON_DEPTH)];

static int writeToArduinoSerial(jsonlite_stream stream, const void *data, size_t length) {
    Serial.write(reinterpret_cast<const char *>(data), length);
    return length;
}

void setup() {
  Serial.begin(9600);
    
  context ctx;
  jsonlite_stream_struct stream;
  stream.write = writeToArduinoSerial;

  ctx.builder = jsonlite_builder_init(builder_memory, sizeof(builder_memory), &stream);
  jsonlite_builder_set_indentation(ctx.builder, 4);

  ctx.parser = jsonlite_parser_init(parser_memory, sizeof(parser_memory), jsonlite_null_buffer());
  jsonlite_parser_callbacks cbs;
  cbs.event_occurred = event_occurred;
  cbs.token_found = token_callback;
  cbs.context.client_state = &ctx;

  jsonlite_parser_set_callback(ctx.parser, &cbs);
  jsonlite_parser_tokenize(ctx.parser, json, sizeof(json));
}

void loop() {
}

