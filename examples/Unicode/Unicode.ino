#include <jsonlite.h>

#define MAX_CHUNK_SIZE 32
#define MAX_JSON_DEPTH 4
#define MAX_JSON_TOKEN_SIZE 128

typedef struct {
  const wchar_t *text;
  const char *answer;
} string_pair;

const char json[] = "[\"\\u041F\\u0440\\u0438\\u0432\\u0456\\u0442\"]";
uint8_t parserMemory[jsonlite_parser_estimate_size(MAX_JSON_DEPTH)];
uint8_t bufferMemory[jsonlite_buffer_static_size_ext(MAX_JSON_TOKEN_SIZE, MAX_CHUNK_SIZE)];
uint8_t chunkBuffer[MAX_CHUNK_SIZE];
string_pair answers[] = {
  {L"Привіт", "Hello in Ukrainian language"}, // use ["\u041F\u0440\u0438\u0432\u0456\u0442"]
  {L"こんにちは", "Hello in Japanese language"}, // use ["\u3053\u3093\u306B\u3061\u306F"]
  {L"გამარჯობა", "Hello in Georgian language"} // use ["\u10D2\u10D0\u10DB\u10D0\u10E0\u10EF\u10DD\u10D1\u10D0"]
  // Place "Hello" in your native language here
};

int answersCount = sizeof(answers) / sizeof(answers[0]);
size_t chunkLength = 0;

jsonlite_parser parser;
jsonlite_buffer buffer;
jsonlite_result result;
wchar_t utf16[128];

static int my_wcscmp(const wchar_t *string1, const wchar_t *string2) {
  while (*string1 && *string2) {
    if (*string1 != *string2) {
      return 1;
    }

    *string1++;
    *string2++;
  }

  return *string1 == *string2 ? 0 : 1;
}

static void stringCallback(jsonlite_callback_context *ctx, jsonlite_token *token) {
  jsonlite_token_type type = token->type & jsonlite_token_type_mask;

  Serial.println(type);
  if (type != jsonlite_token_string && type != jsonlite_token_key) {
    return;
  }

  auto size = jsonlite_token_size_of_uft16(token);
  if (size >= sizeof(utf16)) {
    return;
  }

  jsonlite_token_to_uft16(token, utf16);

  string_pair *result = nullptr;
  for (int i = 0; i < answersCount; i++) {
    string_pair *sp = answers + i;
    int r = my_wcscmp(sp->text, utf16);
    if (r == 0) {
      result = sp;
      break;
    }
  }

  if (result != nullptr) {
    Serial.println(result->answer);
  }
}

void resetParser() {
  buffer = jsonlite_buffer_static_init(bufferMemory, sizeof(bufferMemory));
  parser = jsonlite_parser_init(parserMemory, sizeof(parserMemory), buffer);

  jsonlite_parser_callbacks cbs;
  jsonlite_parser_callbacks_init(&cbs);
  cbs.token_found = &stringCallback;
  jsonlite_parser_set_callback(parser, &cbs);
}

void setup() {
  resetParser();
  Serial.begin(9600);

  Serial.println("Please send any JSON object using serial terminal.");
  Serial.println("For example: ");
  Serial.println(json);
}

void loop() {
  while (Serial.available() && chunkLength < MAX_CHUNK_SIZE) {
    chunkBuffer[chunkLength++] = (uint8_t)Serial.read();
  }

  if (chunkLength == 0) {
    return;
  }

  result = jsonlite_parser_tokenize(parser, chunkBuffer, chunkLength);

  switch (result) {
    case jsonlite_result_ok:
      resetParser();
      break;
    case jsonlite_result_end_of_stream:
      break;
    default: {
        resetParser();
        Serial.print("Error: ");
        Serial.println((int)result);

        Serial.println("Here V");
        Serial.print("     ");
        Serial.write(parser->cursor, parser->limit - parser->cursor);
        Serial.println();
        break;
      }
  }

  chunkLength = 0;
}

