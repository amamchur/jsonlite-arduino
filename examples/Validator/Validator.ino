#include <jsonlite.h>

#define MAX_JSON_DEPTH 4

const char json[] = "[-1, 0, 1, true, false, null]";
uint8_t parser_memory[jsonlite_parser_estimate_size(MAX_JSON_DEPTH)];

void setup() {
  Serial.begin(9600);
  
  size_t mem_used = jsonlite_parser_estimate_size(MAX_JSON_DEPTH);
  Serial.print("jsonlite uses ");
  Serial.print(mem_used);
  Serial.println(" bytes of RAM for JSON validation");

  jsonlite_parser p = jsonlite_parser_init(parser_memory, sizeof(parser_memory), jsonlite_null_buffer());
  jsonlite_result result = jsonlite_parser_tokenize(p, json, sizeof(json));
  
  Serial.print("Result: ");
  Serial.println((int)result);
}

void loop() {
}

