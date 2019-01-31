#include <jsonlite.h>

#define MAX_CHUNK_SIZE 64
#define MAX_JSON_DEPTH 4
#define MAX_JSON_TOKEN_SIZE 20

const char json[] = "[-13453, 0, 1, 123, 45345, -94555]";
uint8_t parserMemory[jsonlite_parser_estimate_size(MAX_JSON_DEPTH)];
uint8_t bufferMemory[jsonlite_buffer_static_size_ext(MAX_JSON_TOKEN_SIZE, MAX_CHUNK_SIZE)];
uint8_t chunkBuffer[MAX_CHUNK_SIZE];
size_t chunkLength = 0;

jsonlite_parser parser;
jsonlite_buffer buffer;
jsonlite_result result;
long total_sum = 0;

static void number_callback(jsonlite_callback_context *ctx, jsonlite_token *token) {
  jsonlite_token_type type = token->type & jsonlite_token_type_mask;
  if (type == jsonlite_token_number) {
    total_sum += jsonlite_token_to_long(token);
  }
}

void resetParser() {
  buffer = jsonlite_buffer_static_init(bufferMemory, sizeof(bufferMemory));
  parser = jsonlite_parser_init(parserMemory, sizeof(parserMemory), buffer);

  jsonlite_parser_callbacks cbs;
  jsonlite_parser_callbacks_init(&cbs);
  cbs.token_found = &number_callback;
  cbs.context.client_state = &total_sum;
  jsonlite_parser_set_callback(parser, &cbs);
}

void setup() {
  Serial.begin(9600);

  resetParser();
  result = jsonlite_parser_tokenize(parser, json, sizeof(json));

  Serial.print("Result: ");
  Serial.println((int)result);
  Serial.print("Total sum: ");
  Serial.println(total_sum);

  total_sum = 0;
  resetParser();
}


void loop() {
  while (Serial.available() && chunkLength < MAX_CHUNK_SIZE) {
    chunkBuffer[chunkLength++] = (uint8_t)Serial.read();
  }

  if (chunkLength == 0) {
    return;
  }

  result = jsonlite_parser_tokenize(parser, chunkBuffer, chunkLength);
  chunkLength = 0;

  switch (result) {
    case jsonlite_result_ok:
      Serial.print("Total sum: ");
      Serial.println(total_sum);
      total_sum = 0;
      resetParser();
      break;
    case jsonlite_result_end_of_stream:
      break;
    default:
      resetParser();
      Serial.print("Error: ");
      Serial.println((int)result);
      break;
  }
}

